#pragma once

#include "framestreamer/udpinterface.hpp"
#include <opencv2/imgcodecs.hpp>

/**
 * Streams frames to the client
 */
class FrameSender : public UdpInterface
{
public:
    /**
     * Fill client address structure and create a socket
     *
     * @param stream_name Title of the stream.
     * @param client_address Ip address of the client to which data will be sent
     * @param client_port Port of the client
     * @param frame_parts_delay Amount of sleep time in microseconds between sending frame parts
     */
    FrameSender(std::string stream_name, std::string client_address, int client_port = 1100,
                unsigned frame_parts_delay = 500)
        : UdpInterface(client_address, client_port), stream_name(stream_name)
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
