#include "framestreamer/udpinterface.hpp"
#include "framestreamer/streamexception.hpp"

#include <unistd.h>

UdpInterface::UdpInterface(std::string server_ip, int server_port)
{
    // Create server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port);
    serverAddr.sin_addr.s_addr = (server_ip == "" ? htonl(INADDR_ANY) : inet_addr(server_ip.c_str()));

    // Create the socket
    mySocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (mySocket == -1)
    {
        throw StreamException("Cannot create socket", errno);
    }
}

UdpInterface::~UdpInterface() { close(mySocket); }
