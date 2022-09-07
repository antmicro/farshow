#include "framestreamer/streamexception.hpp"
#include "framestreamer/utils.hpp"

#include <framestreamer/framesender.hpp>
#include <iostream>
#include <libv4l2.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

#include <camera-capture/cameracapture.hpp>
#include <camera-capture/frameconverters/raw2yuvconverter.hpp>
#include <opencv2/imgproc.hpp>

/**
 * Client is e.g. and embeeded device. It streams the frames
 */

int main()
{
    v4l2_open("asfd", 02); // BUG: it's needed to make v4l2 work with cmake (needed for camera-capture)
    int width = 960;
    int height = 720;
    CameraCapture camera = CameraCapture("/dev/video0");
    camera.setFormat(width, height, V4L2_PIX_FMT_YYUV);
    std::shared_ptr<FrameConverter> converter = std::make_shared<Raw2YuvConverter>(cv::COLOR_YUV2BGR_YUY2); 
    camera.setConverter(converter);
    cv::Mat frame;

    FrameSender streamer = FrameSender("test", "127.0.0.1");

    // streamer.sendFrame(frame, ".png", {cv::IMWRITE_PNG_COMPRESSION, 0});
    while (1)
    {
        frame = camera.capture(CV_8UC2);
        streamer.sendFrame(frame, ".jpg", {cv::IMWRITE_JPEG_QUALITY, 60});
        usleep(41666);
    }
    return 0;
}
