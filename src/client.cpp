#include "framestreamer/framereceiver.hpp"
#include "framestreamer/streamexception.hpp"

#include "cxxopts/cxxopts.hpp"
#include <thread>
#include <future>
#include <chrono>
#include "framestreamer/utils.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std::chrono_literals; // for ms

/**
 * Client receives and shows streams
 */

//------------------ COMMAND LINE OPTIONS ----------------------
/**
 * Configuration of current program execution
 */
typedef struct Config
{
    std::string ip = ""; ///< my ip address
    int port = 1100;     ///< my port
} Config;

/**
 * Parse command line options
 *
 * @param argc Arguments counter
 * @param argv Arguments values
 */
Config parseOptions(int argc, char const *argv[])
{
    Config config;
    cxxopts::ParseResult result;

    // Set available options
    cxxopts::Options options("Frame-streamer client",
                             "A demo for frame-streamer – a minimalistic library to stream frames from e.g. embeeded "
                             "devices.\nClient is receiving and showing frames from a stream.");

    options.add_options()
        ("i, ip", "IP address to which the stream was sent", cxxopts::value(config.ip))
        ("p, port", "Port to which stream was sent", cxxopts::value(config.port)->default_value("1100"))
        ("h, help", "Print usage");

    // Get command line parameters and parse them
    try
    {
        result = options.parse(argc, argv);
    }
    catch (cxxopts::OptionException e)
    {
        std::cerr << std::endl
                  << "\033[31mError while parsing command line arguments: " << e.what() << "\033[0m" << std::endl
                  << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
    }

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    return config;
}

//--------------------------- GUI ------------------------------

static void glfwErrorCallback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

/**
 * Init GLFW and determine glsl version
 */
const char *initGui()
{
    // Setup window
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
    {
        throw StreamException("Cannot initialize glfw");
    }

    const char *glsl_version;
// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    return glsl_version;
}

/**
 * Create window with the size of the screen
 *
 * @param name Window name
 */
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

/**
 * Set context amd backend for Dear ImGui
 */
void setupDearImGui(GLFWwindow *window, const char *glsl_version)
{
    // context
    ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    ImGui::StyleColorsDark();

    // Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

/**
 * Prepare Dear ImGui window with the image
 *
 * @param frame Frame to display
 */
void displayFrame(Frame &frame)
{
    ImGui::Begin(frame.name.c_str());
    ImGui::Image((void *)(intptr_t)frame.texture, ImVec2(frame.img.cols, frame.img.rows));
    ImGui::End();
    std::cout << "Showing " << frame.name << " " << frame.texture << " \n";
}

/**
 * Render prepared frames
 *
 * @param window Window in which frames will be rendered
 */
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

/**
 * Close Dear ImGui and GLFW
 *
 * @param window Window to clean and close
 */
void cleanUp(GLFWwindow *window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(int argc, const char **argv)
{

    Config config = parseOptions(argc, argv); ///< parsed command line arguments
    const char *glsl_version = initGui();
    GLFWwindow *window = createWindow("Frame streamer"); ///< window for displaying the streams
    setupDearImGui(window, glsl_version);

    FrameReceiver receiver = FrameReceiver(config.ip, config.port);
    std::unordered_map<std::string, Frame> frames; ///< Most recent frames from all streams
    Frame frame;
    std::future<Frame> future_frame;

    future_frame = std::async(&FrameReceiver::receiveFrame, &receiver); // new thread will recv frame

    while (!glfwWindowShouldClose(window))
    {
        glfwWaitEvents();

        // Event could be an empty event, raised by the async function when the frame is received
        if(future_frame.wait_for(100ms) == std::future_status::ready)
        {
            frame = future_frame.get(); ///< new frame
            frame.texture = loadTextureFromCVMat(frame.img);
            IM_ASSERT(frame.texture);
            frames[frame.name] = frame;

            future_frame = std::async(&FrameReceiver::receiveFrame, &receiver);
        }
        // If not, other events are processed automatically

        // Display a new frame, to allow moving and resizing windows, even when streams have stopped
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for (auto f: frames)
        {
            displayFrame(f.second);
        }
        render(window);
    }

    shutdown(receiver.getSocket(), 2); // To stop the child thread, blocked on recv
    cleanUp(window);
    return 0;
}
