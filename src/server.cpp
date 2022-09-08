#include "framestreamer/streamexception.hpp"
#include "framestreamer/utils.hpp"

#include <framestreamer/framesender.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

#include <camera-capture/cameracapture.hpp>
#include <camera-capture/frameconverters/raw2yuvconverter.hpp>
#include <opencv2/imgproc.hpp>
#include "cxxopts/cxxopts.hpp"

/**
 * Server is e.g. and embeeded device. It streams the frames
 */
 class ImgTypeInfo
 {
 public:
     ImgTypeInfo(){};
     ImgTypeInfo(std::string extension, int default_quality, int quality_encoding_param): extension(extension), quality(default_quality), quality_encoding_param(quality_encoding_param){}
     std::string extension;
     int quality;
     int quality_encoding_param;

     std::vector<int> getEncodingParams()
     {
        return {quality_encoding_param, quality};
     }
 };

 /**
  * Configuration of current program execution
  */
 typedef struct Config {
     std::string stream_name; ///< stream name
     std::string server_ip;   ///< server's ip address
     int server_port;         ///< server's port
     ImgTypeInfo extension;   ///< Extension of the format in which frames will be send
     std::string source;      ///< Filename of camera device -- stream source
 } Config;

 /**
  * Check if all required arguments were provided
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
  * Parse command line options
  *
  * @param argc Arguments counter
  * @param argv Arguments values
  */
 Config parseOptions(int argc, char const *argv[])
 {
     Config config;
     cxxopts::ParseResult result;

     std::unordered_map <std::string, ImgTypeInfo> img_types = {{".jpg", ImgTypeInfo(".jpg", 65, cv::IMWRITE_JPEG_QUALITY)}, {".png", ImgTypeInfo(".png", 5, cv::IMWRITE_PNG_COMPRESSION)}};

     // Set available options
     cxxopts::Options options("Frame-streamer", "A demo for frame-streamer – a minimalistic library to stream frames from e.g. embeeded devices.");

     options.add_options()
         ("n, name", "Stream name", cxxopts::value(config.stream_name))
         ("i, ip", "IP address of the server, which should receive stream", cxxopts::value(config.server_ip))
         ("p, port", "Port of the server, which will receive stream", cxxopts::value(config.server_port)->default_value("1100"))
         ("e, extension", "Extension of the format in which frames will be send (e.g. `.jpg`, `.png`)", cxxopts::value<std::string>()->default_value(".jpg"))
         ("q, quality", "Frames quality factor (for jpg in range 0 to 100, for png it's compression rate from 0 to 9)", cxxopts::value<int>())
         ("s, source", "Filename of a camera device, which will be a stream source", cxxopts::value(config.source)->default_value("/dev/video0"))
         // ("t, type", "Frame type (allowed values: YUYV, JPG, BGRA, AR24, RGGB, RG12)", cxxopts::value(config.type))
         ("h, help", "Print usage");

     options.positional_help("<stream name> <server ip address>");

     std::vector<std::string> required = {"name", "ip"};

     // Get command line parameters and parse them
     try
     {
         options.parse_positional({"name", "ip"});
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

 int main(int argc, const char **argv)
 {
     Config config = parseOptions(argc, argv);
     // v4l2_open("asfd", 02); // BUG: it's needed to make v4l2 work with cmake (needed for camera-capture)
     CameraCapture camera = CameraCapture(config.source);
     std::shared_ptr<FrameConverter> converter = std::make_shared<Raw2YuvConverter>(cv::COLOR_YUV2BGR_YUY2);//TODO
     camera.setConverter(converter);
     cv::Mat frame;

     FrameSender streamer = FrameSender(config.stream_name, config.server_ip, config.server_port);

     std::cout << config.extension.extension << " " << config.extension.quality << std::endl;
     while (1)
     {
         frame = camera.capture(CV_8UC2);

         streamer.sendFrame(frame, config.extension.extension, config.extension.getEncodingParams());
         usleep(41666);
     }
 }
