#include "framestreamer/framestreamer.hpp"
#include "framestreamer/streamexception.hpp"
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

int main(int argc, char **argv)
{
    FrameStreamer streamer = FrameStreamer("127.0.0.1", 1101);
    streamer.stream_name = "test";

    cv::Mat frame = cv::imread("../test_image.jpg");
    if (frame.empty())
    {
        throw StreamException("Cannot read image\n");
    }

    streamer.sendFrame(frame);
}
