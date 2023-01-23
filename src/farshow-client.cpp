#include "farshow/streamexception.hpp"

#include "farshow/client.hpp"

#include "cxxopts/cxxopts.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <opencv2/imgproc.hpp>
#include <thread>

#define GLSL_VERSION "#version 130"

/**
 * Configuration of current program execution
 */
typedef struct Config
{
    std::string ip = ""; ///< my ip address
    int port = 1100;     ///< my port
} Config;

std::unordered_map<std::string, farshow::FrameWindow> frames; ///< Most recent frames from all streams
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
    farshow::FrameReceiver receiver(config.ip, config.port);
    socket_id = receiver.getSocket();
    farshow::Frame frame;

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
            frames.insert({frame.name, farshow::FrameWindow(frame)});
        }
        frames_mutex.unlock();
        glfwPostEmptyEvent(); // to unblock parent thread

        frame = receiver.receiveFrame();
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

int main(int argc, const char **argv)
{
    static Config config = parseOptions(argc, argv); ///< parsed command line arguments

    farshow::initGui();
    std::thread receiver_thread = std::thread(receiveFrames, config); ///< thread receiving messages from servers
    GLFWwindow *window = farshow::createWindow("farshow");            ///< window for displaying the streams
    farshow::setupDearImGui(window);

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

        farshow::render(window);
    }

    std::cout << "Closing client...\n";
    shutdown(socket_id, 2); // To stop the child thread, blocked on recv
    receiver_thread.join();
    farshow::cleanUp(window);
    return 0;
}
