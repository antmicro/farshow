#include "framestreamer/framestreamer.hpp"

class FrameSender : public FrameStreamer
{
public:
    /**
     * Fill server address structure and create a socket
     *
     * @param server_ip Ip address of the server to which data will be sent
     * @param server_port Port of the server
     */
    FrameSender(std::string server_address, int server_port = 1100) : FrameStreamer(server_address, server_port) {}

    /**
     * Encode the frame and send it (in parts if it's too big to fit the datagram).
     *
     * @param frame Frame to send
     */
    void sendFrame(cv::Mat frame);

    std::string stream_name; ///< name of the stream
};
