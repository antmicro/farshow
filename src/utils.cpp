#include "framestreamer/utils.hpp"
#include <opencv2/highgui.hpp>

/**
 * Display a window with the image
 */
void showImage(cv::Mat frame, std::string winName)
{
    namedWindow(winName, cv::WINDOW_AUTOSIZE); // Create a window
    imshow(winName, frame);
    cv::waitKey(0);
}
