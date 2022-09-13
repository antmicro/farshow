#include "framestreamer/udpinterface.hpp"
#include "framestreamer/streamexception.hpp"

#include <unistd.h>

UdpInterface::UdpInterface(std::string client_ip, int client_port)
{
    // Create client address structure
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(client_port);
    clientAddr.sin_addr.s_addr = (client_ip == "" ? htonl(INADDR_ANY) : inet_addr(client_ip.c_str()));

    // Create the socket
    mySocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (mySocket == -1)
    {
        throw StreamException("Cannot create socket", errno);
    }
}

UdpInterface::~UdpInterface() { close(mySocket); }
