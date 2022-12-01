#include "framestreamer/streamexception.hpp"
#include <csignal>
#include <framestreamer/framesender.hpp>

#include "cxxopts/cxxopts.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

/**
 * Server is e.g. and embeeded device. It streams the frames
 */

bool running; ///< if the server should be running

/**
 * Information about image type in which frames will be streamed
 */
class ImgTypeInfo
{
public:
    /**
     * Default constructor
     */
    ImgTypeInfo(){};

    /**
     * Constructor
     *
     * @param extension Extension of the format in which frames will be send (e.g. `.jpg`, `.png`)
     * @param default_quality Frames quality factor (e.g. for jpg in range 0 to 100, for png it's compression rate from
     * 0 to 9)
     * @param quality_encoding_param Enum from openCV determining which parameter to change see:
     * https://docs.opencv.org/3.4/d8/d6a/group__imgcodecs__flags.html#ga292d81be8d76901bff7988d18d2b42ac
     */
    ImgTypeInfo(std::string extension, int default_quality, int quality_encoding_param)
        : extension(extension), quality(default_quality), quality_encoding_param(quality_encoding_param)
    {
    }

    /**
     * Provides vector with encoding parameters, ready to pass to cv::imencode
     *
     * @returns vector with encoding parameters, ready to pass to cv::imencode
     */
    std::vector<int> getEncodingParams() { return {quality_encoding_param, quality}; }

    std::string extension; ///< Extension of the format in which frames will be send (e.g. `.jpg`, `.png`)
    int quality; ///< Frames quality factor (for jpg in range 0 to 100, for png it's compression rate from 0 to 9)
    int quality_encoding_param; ///< Enum from openCV determining which parameter to change see:
                                ///< https://docs.opencv.org/3.4/d8/d6a/group__imgcodecs__flags.html#ga292d81be8d76901bff7988d18d2b42ac
};

/**
 * Configuration of current program execution
 */
typedef struct Config
{
    std::string stream_name; ///< stream name
    std::string client_ip;   ///< client's ip address
    int client_port;         ///< client's port
    ImgTypeInfo extension;   ///< extension of the format in which frames will be send
    std::string source;      ///< filename of camera device -- stream source
} Config;

/**
 * Checks if all required arguments were provided
 *
 * @param result Parsed arguments
 * @param required Required arguments
 */
void checkRequiredArgs(cxxopts::ParseResult &result, std::vector<std::string> &required)
{
    for (auto &r : required)
    {
        if (result.count(r) == 0)
        {
            throw StreamException("Error while parsing command line arguments: Parameter '" + r + "' is required");
        }
    }
}

/**
 * Parses command line options
 *
 * @param argc Arguments counter
 * @param argv Arguments values
 *
 * @returns Parsed configuration
 */
Config parseOptions(int argc, char const *argv[])
{
    Config config;
    cxxopts::ParseResult result;

    std::unordered_map<std::string, ImgTypeInfo> img_types = {
        {".jpg", ImgTypeInfo(".jpg", 65, cv::IMWRITE_JPEG_QUALITY)},
        {".png", ImgTypeInfo(".png", 5, cv::IMWRITE_PNG_COMPRESSION)}};

    // Set available options
    cxxopts::Options options("Frame-streamer server",
                             "A demo for frame-streamer – a minimalistic library to stream frames from e.g. embeeded "
                             "devices.\nServer is capturing and streaming the frames.");

    // clang-format off
    options.add_options()
        ("i, ip", "IP address of the client, which should receive stream. To send to multiple clients, enter broadcast address", cxxopts::value(config.client_ip))
        ("p, port", "Port of the client, which will receive stream",
                cxxopts::value(config.client_port)->default_value("1100"))
        ("e, extension", "Extension of the format in which frames will be send (e.g. `.jpg`, `.png`)",
                cxxopts::value<std::string>()->default_value(".jpg"))
        ("q, quality", "Frames quality factor (for jpg in range 0 to 100, for png it's compression rate from 0 to 9)",
                cxxopts::value<int>())
        ("s, source", "Filename of a camera device, which will be a stream source",
                cxxopts::value(config.source)->default_value("/dev/video0"))
        ("h, help", "Print usage");
    // clang-format on

    options.positional_help("<client ip address>");

    std::vector<std::string> required = {"ip"};

    // Get command line parameters and parse them
    try
    {
        options.parse_positional({"ip"});
        result = options.parse(argc, argv);
        checkRequiredArgs(result, required);
        config.extension = img_types[result["extension"].as<std::string>()];
    }
    catch (cxxopts::OptionException e)
    {
        std::cerr << std::endl
                  << "\033[31mError while parsing command line arguments: " << e.what() << "\033[0m" << std::endl
                  << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
    }
    catch (StreamException e)
    {
        std::cerr << "\033[31m" << e.what() << "\033[0m" << std::endl << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
    }

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.count("quality"))
    {
        std::cout << "quality" << result["quality"].as<int>() << std::endl;
        config.extension.quality = result["quality"].as<int>();
    }

    return config;
}

void signalHandler(int signum)
{
    std::cout << "Closing server..." << std::endl;
    running = false;
}

int main(int argc, const char **argv)
{
    running = true;
    signal(SIGINT, signalHandler); ///< exit with ^C
    Config config = parseOptions(argc, argv);
    cv::VideoCapture cap(config.source);
    cv::Mat frame;
    cv::Mat gray_frame;

    FrameSender streamer = FrameSender(config.client_ip, config.client_port);

    while (running)
    {
        cap >> frame;
        if (frame.empty())
        {
            break;
        }
        streamer.sendFrame(frame, "input", config.extension.extension, config.extension.getEncodingParams());

        cv::blur(frame, frame, cv::Size2i(7, 7));
        streamer.sendFrame(frame, "blur", config.extension.extension, config.extension.getEncodingParams());

        cv::cvtColor(frame, gray_frame, cv::COLOR_RGB2GRAY);
        cv::adaptiveThreshold(gray_frame, gray_frame, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);
        streamer.sendFrame(gray_frame, "threshold", config.extension.extension, config.extension.getEncodingParams());
    }
}
