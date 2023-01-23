#pragma once
#include "farshow/framereceiver.hpp"
#include "imgui/imgui.h"
#include <GL/glew.h> //GLuint
#include <GLFW/glfw3.h>

namespace farshow
{

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
    FrameWindow(Frame &frame) : name(frame.name), changed(true), texture(-1) { changeImg(frame.img); }

    /**
     * Replaces img and marks it as changed
     *
     * The method can be launched from different thread, so the reloadTexture function is not used. You have to run it
     * manually from the main thread.
     *
     * If the image has 1 channel it's assumed to be grayscale (and it's converted to BGR), otherwise it's treated like
     * BGR.
     *
     * @param new_image New image
     */
    void changeImg(cv::Mat &new_image);

    /**
     * Reloads the texture from the img. Creates a texture handler if it's not present.
     * Image should be in BGR format.
     *
     * Has to be run from the same thread as `glfwInit` (the main one)
     */
    void reloadTexture();

    /**
     * Prepares Dear ImGui window with the image
     */
    void display();

    ~FrameWindow();

private:
    GLuint texture = -1;  ///< OpenGL texture identifier
    std::string name;     ///< Window name
    cv::Mat img;          ///< Image to display
    bool changed = false; ///< If the img has changed since last texture reload
    struct WindowData     ///< Window options
    {
        float aspect_ratio;
        ImVec2 offset;
    };

    /**
     * Resizes the window with given aspect ratio.
     *
     * @param data ImGuiSizeCallbackData structure with aspect ratio set in UserData.
     */
    static void aspectRatio(ImGuiSizeCallbackData *data);
};

static void glfwErrorCallback(int error, const char *description);

/**
 * Initializes GLFW
 */
void initGui();

/**
 * Creates window with the size of the screen
 *
 * @param name Window name
 *
 * @returns window
 */
GLFWwindow *createWindow(std::string name);

/**
 * Sets context and backend for Dear ImGui
 *
 * @param window Window on which imgui should operate
 */
void setupDearImGui(GLFWwindow *window);

/**
 * Renders prepared frames
 *
 * @param window Window in which frames will be rendered
 */
void render(GLFWwindow *window);

/**
 * Closes Dear ImGui and GLFW
 *
 * @param window Window to clean and close
 */
void cleanUp(GLFWwindow *window);
}; // namespace farshow
