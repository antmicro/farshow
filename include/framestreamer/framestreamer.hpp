#pragma once

#include <arpa/inet.h> //sockaddr_in
#include <string>

class FrameStreamer
{
public:
    /**
     * Fill address structures, create socket and bind it.
     */
    FrameStreamer(std::string clientIp, int clientPort, int myPort = 1100, std::string myAddress = "");

    /*
     * Close the socket
     */
    ~FrameStreamer();

    void sendMessage(std::string text);
    void receiveMessage();

    struct sockaddr_in myAddress;
    struct sockaddr_in clientAddress;
    int mySocket;
};
