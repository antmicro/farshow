#include "framestreamer/framestreamer.hpp"

class FrameReceiver : public FrameStreamer
{
public:
    /**
     * Fill address structures, create socket and bind it.
     *
     * @param client_ip Client's ip address
     * @param client_port Client's port
     * @param server_port Server's port
     * @param server_ip Server's ip address (if not provided, binds the socket to all available interfaces)
     */
    FrameReceiver(std::string client_ip, int client_port, int my_port = 1100, std::string my_address = "") : FrameStreamer(client_ip, client_port, my_port, my_address) {}

    /**
     * Receive and display the frame
     */
    void receiveFrame();
};
