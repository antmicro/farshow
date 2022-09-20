#pragma once
#include <GL/glew.h>
#include <opencv2/core/mat.hpp>
#include <string>

/**
 * Displays a window with the image
 *
 * @param frame Image to display
 * @param winName Title for the window
 */
void showImage(cv::Mat frame, std::string winName);

/**
 * Loads an image into a OpenGL texture with common settings
 *
 * @param mat Matrix to read data from
 *
 * @returns Name of generated texture
 */
GLuint loadTextureFromCVMat(cv::Mat &mat);
