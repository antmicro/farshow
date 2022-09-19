#include "framestreamer/framereceiver.hpp"
#include "framestreamer/streamexception.hpp"

#include "cxxopts/cxxopts.hpp"
#include "framestreamer/utils.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <signal.h>
#include <thread>

#define GLSL_VERSION "#version 130"


/**
 * Client receives and shows streams
 */


/**
 * Class for handling displaying frames and loading their textures
 */
class FrameWindow
{
public:
    /**
     * Default, empty constructor
     */
    FrameWindow(){}

    /**
     * Constructor. Sets name and image
     *
     * @param frame Frame structure – source of the name and image
     */
    FrameWindow(Frame &frame): name(frame.name), img(frame.img), changed(true){}

    /**
     * Replaces img and marks it as changed
     *
     * The method can be launched from different thread, so the reloadTexture function is not used. You have to run it manually from the main thread.
     *
     * @param new_image New image
     */
    void changeImg(cv::Mat &new_image);

    /**
     * Reloads the texture from the img. Creates a texture handler if it's not present.
     * If the image has 1 channel it's assumed to be grayscale (and it's converted to BGR), otherwise it's treated like BGR.
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
    GLuint texture = -1; ///< OpenGL texture identifier
    std::string name; ///< Window name
    cv::Mat img; ///< Image to display
    bool changed = false; ///< If the img has changed since last texture reload
};

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

    ImGui::Begin(name.c_str(), NULL,  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    //TODO: resize window
    // int y = ImGui::GetWindowSize().x * frame.img.rows/ frame.img.cols;

    // ImGui::Image((void *)(intptr_t)frame.texture, ImVec2(ImGui::GetWindowSize().x, y));
    // ImGui::SetWindowSize(frame.name.c_str(), ImVec2(ImGui::GetWindowSize().x, y));
    ImGui::Image((void *)(intptr_t)texture, ImVec2(img.cols, img.rows));
    ImGui::End();
}

FrameWindow::~FrameWindow()
{
    glDeleteTextures(1, &texture);
}


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
 * Initialize GLFW
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
 * Create window with the size of the screen
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
 * Set context amd backend for Dear ImGui
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

std::unordered_map<std::string, FrameWindow> frames; ///< Most recent frames from all streams
std::mutex frames_mutex; ///< Mutex for `frames` map. The map is used by main and receiver thread
int socket_id; ///< Socket on which messages from servers are received

/**
 * Receive frames and put them in the map
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
        catch(std::out_of_range e)
        {
            frames.insert({frame.name, FrameWindow(frame)});
        }
        frames_mutex.unlock();

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
