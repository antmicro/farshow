#pragma once
#include "framestreamer/framereceiver.hpp"
#include <GL/glew.h> //GLuint

/**
 * Configuration of current program execution
 */
typedef struct Config
{
    std::string ip = ""; ///< my ip address
    int port = 1100;     ///< my port
} Config;

/**
 * Class for handling displaying frames and loading their textures
 */
class FrameWindow
{
public:
    /**
     * Default, empty constructor
     */
    FrameWindow() {}

    /**
     * Constructor. Sets name and image
     *
     * @param frame Frame structure – source of the name and image
     */
    FrameWindow(Frame &frame) : name(frame.name), img(frame.img), changed(true) {}

    /**
     * Replaces img and marks it as changed
     *
     * The method can be launched from different thread, so the reloadTexture function is not used. You have to run it
     * manually from the main thread.
     *
     * @param new_image New image
     */
    void changeImg(cv::Mat &new_image);

    /**
     * Reloads the texture from the img. Creates a texture handler if it's not present.
     * If the image has 1 channel it's assumed to be grayscale (and it's converted to BGR), otherwise it's treated like
     * BGR.
     *
     * Has to be run from the same thread as `glfwInit` (the main one)
     */
    void reloadTexture();

    /**
     * Prepares Dear ImGui window with the image
     *
     * @param frame Frame to display
     */
    void display();

    ~FrameWindow();

private:
    GLuint texture = -1;  ///< OpenGL texture identifier
    std::string name;     ///< Window name
    cv::Mat img;          ///< Image to display
    bool changed = false; ///< If the img has changed since last texture reload
};
