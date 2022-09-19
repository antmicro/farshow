#pragma once
#include <GL/glew.h>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>

/**
 * Displays a window with the image
 *
 * @param frame Image to display
 * @param winName Title for the window
 */
void showImage(cv::Mat frame, std::string winName)
{
    namedWindow(winName, cv::WINDOW_AUTOSIZE); // Create a window
    imshow(winName, frame);
    cv::waitKey(0);
}

/**
 * Loads an image into a OpenGL texture with common settings
 *
 * @param mat Matrix to read data from
 */
GLuint loadTextureFromCVMat(cv::Mat &mat)
{
    GLuint texture;
    // Create a OpenGL texture identifier
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non
                                                                         // power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mat.cols, mat.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, mat.data);

    return texture;
}
