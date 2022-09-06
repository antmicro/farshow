#pragma once
#include "framestreamer/udpinterface.hpp"

class FrameReceiver : public UdpInterface
{
public:
    /**
     * Fill server addres structure, create a socket and bind it.
     *
     * @param server_port Server's port
     * @param server_ip Server's ip address (if not provided, binds the socket to all available interfaces)
     */
    FrameReceiver(std::string server_address = "", int server_port = 1100);

    /**
     * Receive and display the frame
     */
    void receiveFrame();
};
