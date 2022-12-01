#include "farshow/udpinterface.hpp"
#include "farshow/streamexception.hpp"

#include <unistd.h>

namespace farshow
{

UdpInterface::UdpInterface(std::string client_ip, int client_port)
{
    // Create client address structure
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(client_port);
    clientAddr.sin_addr.s_addr = (client_ip == "" ? htonl(INADDR_ANY) : inet_addr(client_ip.c_str()));

    // Create the socket
    mySocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mySocket == -1)
    {
        throw StreamException("Cannot create socket", errno);
    }
}

UdpInterface::~UdpInterface() { close(mySocket); }

};
