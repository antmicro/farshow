#include "framestreamer/framestreamer.hpp"
#include "framestreamer/streamexception.hpp"

#include <iostream>
#include <unistd.h>

FrameStreamer::FrameStreamer(std::string clientIp, int clientPort, int myPort, std::string myIp)
{
    myAddress.sin_family = AF_INET;
    myAddress.sin_port = htons(myPort);
    myAddress.sin_addr.s_addr = (myIp == "" ? htonl(INADDR_ANY) : inet_addr(myIp.c_str()));

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(clientPort);
    clientAddress.sin_addr.s_addr = inet_addr(clientIp.c_str());

    mySocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (mySocket == -1)
    {
        throw StreamException("Cannot create socket", errno);
    }

    if (bind(mySocket, (struct sockaddr *)&myAddress, sizeof(myAddress)) == -1)
    {
        close(mySocket);
        throw StreamException("Cannot bind", errno);
    }

    // connect can also be used for UDP, it will set given adress as default
    // if(connect(localSocket,(struct sockaddr
    // *)&serverAdress,sizeof(serverAdress))<0)
    // {
    //     close(localSocket);
    //     perror("could not connect");
    // }
}

void FrameStreamer::sendMessage(std::string text)
{
    if (sendto(mySocket, text.c_str(), sizeof(text), 0, (const struct sockaddr *)&clientAddress,
               sizeof(clientAddress)) < 0)
    {
        close(mySocket);
        throw StreamException("Cannot send", errno);
    }
    else
    {
        std::cout << "sent : " << text << std::endl;
    }
}

void FrameStreamer::receiveMessage()
{
    int n;
    char buff[256];
    unsigned int len = sizeof(clientAddress);

    if (recvfrom(mySocket, (char *)buff, 256, MSG_WAITALL, (struct sockaddr *)&clientAddress, &len) < 0)
    {
        close(mySocket);
        throw StreamException("Cannot receive message", errno);
    }
    std::cout << "recived: " << buff << std::endl;
}

FrameStreamer::~FrameStreamer() { close(mySocket); }
