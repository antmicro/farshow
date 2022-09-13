#include "framestreamer/streamexception.hpp"
#include "framestreamer/utils.hpp"

#include <framestreamer/framesender.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

#include <camera-capture/cameracapture.hpp>
#include <camera-capture/frameconverters/raw2yuvconverter.hpp>
#include <opencv2/imgproc.hpp>

/**
 * Server is e.g. and embeeded device. It streams the frames
 */

int main(int argc, char **argv)
{
    int width = 960;
    int height = 720;
    CameraCapture camera = CameraCapture("/dev/video0");
    camera.setFormat(width, height, V4L2_PIX_FMT_YYUV);
    std::shared_ptr<FrameConverter> converter = std::make_shared<Raw2YuvConverter>(cv::COLOR_YUV2BGR_YUY2);
    camera.setConverter(converter);
    cv::Mat frame;

    FrameSender streamer = FrameSender(argv[1], "127.0.0.1");

    while (1)
    {
        frame = camera.capture(CV_8UC2);
        streamer.sendFrame(frame, ".jpg", {cv::IMWRITE_JPEG_QUALITY, 95});
        usleep(41666);
    }
    return 0;
}
