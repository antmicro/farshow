#include "framestreamer/streamexception.hpp"
#include <framestreamer/framesender.hpp>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

/**
 * Client is eg. and embeeded device. It streams the frames
 */

int main()
{
    FrameSender streamer = FrameSender("test", "127.0.0.1");
    cv::Mat frame = cv::imread("../test_image.jpg");
    if (frame.empty())
    {
        throw StreamException("Cannot read image\n");
    }

    streamer.sendFrame(frame, ".png", {cv::IMWRITE_PNG_COMPRESSION, 0});
    // streamer.sendFrame(frame, ".jpg", {cv::IMWRITE_JPEG_QUALITY, 100});
    return 0;
}
