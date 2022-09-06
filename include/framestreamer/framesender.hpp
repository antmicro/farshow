#pragma once

#include "framestreamer/udpinterface.hpp"
#include <opencv2/imgcodecs.hpp>

class FrameSender : public UdpInterface
{
public:
    /**
     * Fill server address structure and create a socket
     *
     * @param stream_name Title of the stream.
     * @param server_ip Ip address of the server to which data will be sent
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
     */
    void sendFrame(cv::Mat frame);

    std::string stream_name; ///< name of the stream

    unsigned frame_parts_delay = 500; ///< Amount of sleep time in microseconds between sending frame parts
};
