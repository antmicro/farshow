#pragma once

#include "framestreamer/udpinterface.hpp"
#include <opencv2/imgcodecs.hpp>

/**
 * Streams frames to the server
 */
class FrameSender : public UdpInterface
{
public:
    /**
     * Fill server address structure and create a socket
     *
     * @param stream_name Title of the stream.
     * @param server_address Ip address of the server to which data will be sent
     * @param server_port Port of the server
     */
    FrameSender(std::string stream_name, std::string server_address, int server_port = 1100)
        : UdpInterface(server_address, server_port), stream_name(stream_name)
    {
    }

    /**
     * Encode the frame and send it (in parts if it's too big to fit the datagram).
     *
     * @param frame Frame to send
     * @param extension Extension determining output format (`.jpg`, `.png` ...)
     * @param encoding_params Format-specific parameters for cv::imencode
     */
    void sendFrame(cv::Mat frame, std::string extension = ".jpg",
                   std::vector<int> encoding_params = {cv::IMWRITE_JPEG_QUALITY, 95});

    std::string stream_name; ///< name of the stream

    unsigned frame_parts_delay = 500; ///< Amount of sleep time in microseconds between sending frame parts
private:
    unsigned curr_frame_id = 0; ///< Id for the next frame
};
