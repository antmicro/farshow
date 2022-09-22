#include "framestreamer/streamexception.hpp"

#include "framestreamer/client.hpp"

#include "cxxopts/cxxopts.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <opencv2/imgproc.hpp>
#include <thread>

#define GLSL_VERSION "#version 130"

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

void FrameWindow::display()
{
    if (changed)
    {
        reloadTexture();
    }

    ImGui::Begin(name.c_str(), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // resize window
    ImVec2 view = ImGui::GetWindowSize();
    int y = view.x * img.rows / img.cols; ///< scaled image height
    int title_bar_size = 35; // imGui doesn't provide setWindowContentSize method, so the content area is smaller than
                             // the image and the bottom of the image is cut. To avoid this, we create a bigger window

    ImGui::Image((void *)(intptr_t)texture, ImVec2(view.x, y));
    ImGui::SetWindowSize(name.c_str(), ImVec2(view.x, y + title_bar_size));
    ImGui::End();
}

FrameWindow::~FrameWindow()
{
    if (texture != -1)
    {
        glDeleteTextures(1, &texture);
    }
}

//------------------ COMMAND LINE OPTIONS ----------------------
/**
 * Parses command line options
 *
 * @param argc Arguments counter
 * @param argv Arguments values
 *
 * @returns Stucture with parsed configuration
 */
Config parseOptions(int argc, char const *argv[])
{
    Config config;
    cxxopts::ParseResult result;

    // Set available options
    cxxopts::Options options("Frame-streamer client",
                             "A demo for frame-streamer – a minimalistic library to stream frames from e.g. embeeded "
                             "devices.\nClient is receiving and showing frames from a stream.");
    // clang-format off
    options.add_options()
        ("i, ip", "IP address to which the stream was sent", cxxopts::value(config.ip))
        ("p, port", "Port to which stream was sent", cxxopts::value(config.port)->default_value("1100"))
        ("h, help", "Print usage");
    // clang-format on

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
 * Initializes GLFW
 */
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

/**
 * Creates window with the size of the screen
 *
 * @param name Window name
 *
 * @returns window
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
 * Sets context and backend for Dear ImGui
 *
 * @param window Window on which imgui should operate
 */
void setupDearImGui(GLFWwindow *window)
{
    // context
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    // Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
}

/**
 * Renders prepared frames
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
 * Closes Dear ImGui and GLFW
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

std::unordered_map<std::string, FrameWindow> frames; ///< Most recent frames from all streams
std::mutex frames_mutex; ///< Mutex for `frames` map. The map is used by main and receiver thread
int socket_id;           ///< Socket on which messages from servers are received

/**
 * Receives frames and put them in the map
 *
 * The function is ment to run in the separate thread.
 *
 * @param config Configuration from command line arguments
 */
void receiveFrames(Config config)
{
    FrameReceiver receiver = FrameReceiver(config.ip, config.port);
    socket_id = receiver.getSocket();
    Frame frame;

    frame = receiver.receiveFrame();
    while (!frame.img.empty())
    {
        // Place the new frame in the map
        frames_mutex.lock();
        try
        {
            frames.at(frame.name).changeImg(frame.img);
        }
        catch (std::out_of_range e)
        {
            frames.insert({frame.name, FrameWindow(frame)});
        }
        frames_mutex.unlock();
        glfwPostEmptyEvent(); // to unblock parent thread

        frame = receiver.receiveFrame();
    }
}

int main(int argc, const char **argv)
{
    static Config config = parseOptions(argc, argv); ///< parsed command line arguments

    std::thread receiver_thread = std::thread(receiveFrames, config); ///< thread receiving messages from servers

    initGui();
    GLFWwindow *window = createWindow("Frame streamer"); ///< window for displaying the streams
    setupDearImGui(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Display all streams
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        frames_mutex.lock();
        for (auto &f : frames)
        {
            f.second.display();
        }
        frames_mutex.unlock();

        render(window);
    }

    std::cout << "Closing client...\n";
    shutdown(socket_id, 2); // To stop the child thread, blocked on recv
    receiver_thread.join();
    cleanUp(window);
    return 0;
}
