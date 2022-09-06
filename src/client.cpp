#include <framestreamer/framesender.hpp>
#include "framestreamer/streamexception.hpp"
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

int main()
{
    FrameSender streamer = FrameSender("127.0.0.1", 1100, 1101, "127.0.0.1");
    streamer.stream_name = "test";
    cv::Mat frame = cv::imread("../test_image.jpg");
    if (frame.empty())
    {
        throw StreamException("Cannot read image\n");
    }

    streamer.sendFrame(frame);
    return 0;
}
