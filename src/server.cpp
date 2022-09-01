#include "framestreamer/framestreamer.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    FrameStreamer streamer = FrameStreamer("127.0.0.1", 1101);
    streamer.name = "test";

    // streamer.receiveMessage();
    // streamer.sendMessage("~server");

    cv::Mat frame = cv::imread("../test_image.jpg");
    if (frame.empty())
    {
            std::cout << "!!! Failed imread(): image not found" << std::endl;
    // don't let the execution continue, else imshow() will crash.
    }
    std::vector<uchar> buf(65507);

    streamer.sendFrame(frame);
}
