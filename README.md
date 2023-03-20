# farshow

Copyright (c) 2022-2023 [Antmicro](https://www.antmicro.com)

A minimalistic library for streaming and displaying frames from remote devices.

Work with image processing flows (e.g. from camera streams) usually involves analysing images from intermediate steps.
However, when working with e.g. embedded or remote devices, where displaying live images on the monitor is not always possible, the entire image processing flow debugging process may become tedious.

`farshow` provides:

* `farshow-conection` - a library for streaming frames via UDP
* `farshow` - a program for receiving and displaying such frames on PC, providing the user with a live preview from a remote device using OpenGL and [ImGui](https://github.com/ocornut/imgui)

![Client GUI](resources/client.png)

## Building the project

Project requirements:

* [OpenCV](https://opencv.org/releases/)
* [OpenGL](https://www.khronos.org/opengl/wiki/Getting_Started#Downloading_OpenGL)
* [glfw3](https://www.glfw.org/download)
* C++ compiler with C++20 support (g++-12 is recommended).

Additional requirements for Python bindings:
* [NumPy](https://numpy.org/install)
* [pybind11](https://pybind11.readthedocs.io/en/stable/installing.html)
* [cvnp](https://github.com/pthom/cvnp)

To build the project, go to its root directory and execute:
```
cmake -s . -B build
```

To be able to use Python bindings outside of build directory PYTHONPATH environment variable must be adjusted accordingly.

## Running the demo

The project consists of a streaming library and a receiving application. They communicate with each other using a UDP protocol.

### `farshow` server example

After a successful build, you can run the demo inside the `build` directory, e.g.:

```
./farshow-server-example 127.0.0.1
```

This will start the demonstration server, streaming frames to the server running on `127.0.0.1` on the default port (`1100`). By default, `/dev/video0` is taken as a stream source.

By default, the server sends frames in jpg format, with a quality factor of 95. To use e.g. png format with compression 4, add `-e .png -q 4` to the runtime parameters.

You can find more information about available arguments in command-line help:

```
./farshow-server-example --help
```

### The `farshow` application

To receive and display frames, you have to run the client:
```
./farshow
```

After a successful run, a window with named streams should appear.

![Client GUI gif](resources/client.gif)

By default, the client will use port 1100, and receive messages from all available interfaces.

You can also find more information about available arguments in command-line help:
```
./farshow --help
```

## Usage

All below classes are available in the `farshow` namespace.

The core of the library are the `FrameSender` and `FrameReceiver` classes. They both derive from `UdpInterface`.

### Sending frames

You can use `FrameSender` in your program for the embedded device (server).

First, you should create an instance of the frame sender class:

```c++
#include <farshow/framesender.hpp>
farshow::FrameSender streamer("196.168.1.15", 1111);
```

Where `196.168.1.15` is the client address and `1111` its IP port. The constructor is also responsible for creating a socket.

To send a frame under the name "my_stream" use:

```c++
streamer.sendFrame(frame, "my_stream");
```

Where a frame is a [cv::Mat](https://docs.opencv.org/4.x/d3/d63/classcv_1_1Mat.html), to match the client side, the frame should be sent as grayscale or BGR. We chose BGR as a type common in OpenCV.

This will send the frame as a jpg with quality 95. You can send it in other formats, providing the following arguments. E.g., to send it as a png with compression 4, use:

```c++
streamer.sendFrame(frame, "my_stream", ".png", cv::IMWRITE_PNG_COMPRESSION=4);
```

Look for more information about supported formats in [OpenCV image reading and writing documentation](https://docs.opencv.org/3.4/d4/da8/group__imgcodecs.html#ga288b8b3da0892bd651fce07b3bbd3a56).

To ensure continuity of the stream, use the same name for each frame within it.

#### Technical details
To send a frame, we must encode it and check if it fits the datagram. If it does not, it is split into parts. Each frame has an id and number of parts. Each part also has a separate id. The entire structure of the message is available in the [`udpinterface.hpp`](include/farshow/udpinterface.hpp) file as `FrameMessage`. Then the message is sent to the client, which we assigned when creating the instance of `FrameSender`.


### Receiving frames

The receiver is on the client side. Here we have to join the parts of the frame back together and keep the frames in order.

To receive the frame, create an instance of the `FrameReceiver`:

```c++
#include "farshow/framereceiver.hpp"
farshow::FrameReceiver receiver();
```

Without arguments, it binds the socket to all available interfaces, with the default port 1100. You can, of course, provide the client IP address and port.

Then fetch the frame:
```c++
farshow::Frame frame = receiver.receiveFrame();
```
The function returns a `Frame` structure with two fields: `name` (the stream name) and `img` (cv::Mat with the image).

You can then display the frame however you want. E.g.:

```c++
#include <opencv2/highgui.hpp>

cv::namedWindow(frame.name, cv::WINDOW_AUTOSIZE); // Create a window
cv::imshow(frame.name, frame.img);                // Show image in it
cv::waitKey(0);                                   // Wait for a keypress before closing the window
```

#### Technical details

`receiveFrame` is a loop which receives parts of frames from various streams and joins them until any of the frames is complete (contains all parts).
To keep the frames in order, we've created a mapping from a stream name to a linked list of `FrameContainer`s with all stream frames. It's worth noting that the frames in the stream are mostly incomplete because when any of them is complete, we return it immediately. Frames in the list are sorted by id.

When a new part of a frame appears, firstly we find the stream to which it belongs (by name). Then we look at the frame id and (like in insertion sort) look for a proper place for it. Then we copy the data from the frame part to the place where they should be in the actual frame. Since the ids can overflow, the algorithm assumes that when e.g. frame with id 0 comes after 4294967295, it should be placed at the end to ensure stream continuity.

When the frame is complete, we delete all incomplete frames before it (because we have a newer one), decode it and return its name and image (in a `Frame` structure).

[The `farshow` program](src/farshow-client.cpp) uses [Dear ImGui](https://github.com/ocornut/imgui) to display frames. The program has two threads. One is responsible for receiving frames and the main one – for displaying them. They communicate via a map with stream names and their most recent frames.
When the receiver thread receives a new frame, it puts it in the map, changing the most recent image.
When the main thread notices that the frame is changed, it reloads the texture assigned to the frame and the displayed image changes. Look at [Image Loading and Displaying examples with ImGui](https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples) for more information on how Dear ImGui displays photos.

## Usage in Python

All below classes are available in the `farshow` module.

The core of library i.e. `FrameSender` and `FrameReceiver` is accessible from Python. Python functions resemble C++ counterparts as close as possible. For clarity the usage details are below.

### Sending frames

You can use `FrameSender` in your program for the embedded device (server).

First, you should create an instance of the frame sender class:
```python
import farshow
streamer = farshow.FrameSender("196.168.1.15", 1111)
```
Where `196.168.1.15` is the client address adn `1111` is IP port. The constructor is also responsible for creating socket.

To send a frame under the name "my_stream" use:
```python
streamer.sendFrame(frame, "my_stream")
```
Where a frame is a [numpy.ndarray](https://numpy.org/doc/stable/reference/generated/numpy.ndarray.html), to match client side, the frame should be sent OpenCV compatible format, grayscale or BGR. We chose BGR as a type common in OpenCV.

This will send the frame as a jpg with quality 95. You can send it in other formats, providing the following arguments. E.g., to send it as png with compression 4, use:
```python
streamer.sendFrame(frame, "my_stream", ".png", [cv2.IMWRITE_PNG_COMPRESSION, 4])
```

### Receiving frames

The receiver is on client side. Here we have to join the parts of frame back together and keep the frames in order.

To receive the frame, create an instance of `FrameReceiver`:
```python
import farshow
receiver = farshow.FrameReceiver();
```
Without arguments, it binds the socket to all avaible interfaces, with the default port 1100. You can, of course, provide the client IP address and port.

Then fetch the frame:
```python
frame = receiver.receiveFrame()
```
The function returns a `Frame` class with two fields: `name` (the stream name) and `img` (numpy.ndarray with the image).

You can then display the frame however you want. E.g.:
```python
import cv2

cv2.namedWindow(frame.name, cv2.WINDOW_AUTOSIZE) # Create a window
cv2.imshow(frame.name, frame.img)		 # Show image in it
cv2.waitKey(0)					 # Wait for a keypress before closing the window
```

## Licensing

The sources are published under the Apache 2.0 License, except for files located in the `third-party/` directory. For those files, the license is either enclosed in the file header or a separate LICENSE file.

The README video samples were created using Elephant's Dream, released under [Creative Commons Attribution](https://creativecommons.org/licenses/by/2.5/) license:

(c) copyright 2006, Blender Foundation / Netherlands Media Art Institute / www.elephantsdream.org
