#include "framestreamer/framestreamer.hpp"
#include "framestreamer/streamexception.hpp"

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>


FrameStreamer::FrameStreamer(std::string client_ip, int client_port, int server_port, std::string server_ip)
{
    // Create address structures
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port);
    serverAddr.sin_addr.s_addr = (server_ip == "" ? htonl(INADDR_ANY) : inet_addr(server_ip.c_str()));

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(client_port);
    clientAddr.sin_addr.s_addr = inet_addr(client_ip.c_str());

    // Create and bind the socket
    serverSocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1)
    {
        throw StreamException("Cannot create socket", errno);
    }

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        close(serverSocket);
        throw StreamException("Cannot bind", errno);
    }
}

FrameStreamer::~FrameStreamer() { close(serverSocket); }
