#include "farshow/streamexception.hpp"

#include "farshow/client.hpp"

#include "cxxopts/cxxopts.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <opencv2/imgproc.hpp>
#include <thread>

#define GLSL_VERSION "#version 130"

namespace farshow
{

/**
 * Client receives and shows streams
 */
void FrameWindow::reloadTexture()
{
    if (texture == -1)
    {
        glGenTextures(1, &texture);
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
    IM_ASSERT(texture);
    changed = false;
}

void FrameWindow::changeImg(cv::Mat &new_image)
{
    if (new_image.channels() == 1)
    {
        cv::cvtColor(new_image, img, cv::COLOR_GRAY2RGB);
    }
    else
    {
        img = new_image;
    }
    changed = true;
}

void FrameWindow::aspectRatio(ImGuiSizeCallbackData *data)
{
    struct WindowData userdata = *(struct WindowData *)data->UserData;
    float aspect_ratio = userdata.aspect_ratio;
    float diffx = data->CurrentSize.x - data->DesiredSize.x;
    float diffy = data->CurrentSize.y - data->DesiredSize.y;
    int diff = diffx + diffy;
    if (abs(diff) > 3)
    {
        data->DesiredSize.x = data->CurrentSize.x - diff - userdata.offset.x;
        data->DesiredSize.y = data->DesiredSize.x / aspect_ratio;
        // include window padding and title bar in resized window
        data->DesiredSize.x += userdata.offset.x;
        data->DesiredSize.y += userdata.offset.y;
    }
    else
    {
        data->DesiredSize.x = data->CurrentSize.x;
        data->DesiredSize.y = data->CurrentSize.y;
    }
}
void FrameWindow::display()
{
    ImGuiStyle &style = ImGui::GetStyle();
    // imGui doesn't provide setWindowContentSize method, so the content area is smaller than the image and the bottom
    // of the image is cut. To avoid this, we create a bigger window
    const int title_bar_size = style.FramePadding.y * 2 + ImGui::GetFontSize();
    // To reduce flickering, we round the aspect ratio to 1 decimal place
    float aspect_ratio = round((float)img.cols / (float)img.rows * 10) / 10;
    ImVec2 offset = ImVec2(style.WindowPadding.x * 2, style.WindowPadding.y * 2 + title_bar_size);
    struct WindowData window_options = {aspect_ratio, offset};
    if (changed)
    {
        reloadTexture();
        ImGui::SetNextWindowSize(ImVec2(img.cols + offset.x, img.cols / aspect_ratio + offset.y),
                                 ImGuiCond_FirstUseEver);
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX), aspectRatio, (void *)&window_options);

    ImGui::Begin(name.c_str(), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 view = ImGui::GetWindowSize();
    ImGui::Image((void *)(intptr_t)texture, ImVec2(view.x - offset.x, view.y - offset.y));

    ImVec2 pos = ImGui::GetWindowPos();
    ImGui::SetNextWindowPos(ImVec2(pos.x + title_bar_size, pos.y + title_bar_size), ImGuiCond_FirstUseEver);
    ImGui::End();
}

FrameWindow::~FrameWindow()
{
    if (texture != -1)
    {
        glDeleteTextures(1, &texture);
    }
}

//--------------------------- GUI ------------------------------

static void glfwErrorCallback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void initGui()
{
    // Setup window
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
    {
        throw StreamException("Cannot initialize glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
}

GLFWwindow *createWindow(std::string name)
{
    // Create window with graphics context
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, name.c_str(), NULL, NULL);
    if (window == NULL)
    {
        throw StreamException("Cannot create window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    return window;
}

void setupDearImGui(GLFWwindow *window)
{
    // context
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    // Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
}

void render(GLFWwindow *window)
{
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

void cleanUp(GLFWwindow *window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

}; // namespace farshow
