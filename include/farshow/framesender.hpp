#pragma once

#include "farshow/udpinterface.hpp"
#include <opencv2/imgcodecs.hpp>

namespace farshow
{

/**
 * Streams frames to the client
 */
class FrameSender : public UdpInterface
{
public:
    /**
     * Fills client address structure and create a socket
     *
     * @param client_address Ip address of the client to which data will be sent
     * @param client_port Port of the client
     * @param frame_parts_delay Amount of sleep time in microseconds between sending frame parts
     */
    FrameSender(std::string client_address, int client_port = 1100, unsigned frame_parts_delay = 500)
        : UdpInterface(client_address, client_port)
    {
        // Enable broadcasting
        int broadcast = 1;
        setsockopt(mySocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    }

    /**
     * Encodes the frame and send it (in parts if it's too big to fit the datagram).
     * To match the client side, the frame should be send as grayscale or BGR.
     *
     * @param frame Frame to send
     * @param name Title of the stream
     * @param extension Extension determining output format (`.jpg`, `.png` ...)
     * @param encoding_params Format-specific parameters for cv::imencode
     */
    void sendFrame(cv::Mat &frame, std::string name, std::string extension = ".jpg",
                   std::vector<int> encoding_params = {cv::IMWRITE_JPEG_QUALITY, 95});

    unsigned frame_parts_delay = 500; ///< Amount of sleep time in microseconds between sending frame parts
private:
    unsigned curr_frame_id = 0; ///< Id for the next frame
};

};
