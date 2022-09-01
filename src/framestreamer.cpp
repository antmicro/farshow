#include "framestreamer/framestreamer.hpp"
#include "framestreamer/streamexception.hpp"

#include <iostream>
#include <unistd.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>



void showImage(cv::Mat frame, std::string winName)
{
    namedWindow(winName, cv::WINDOW_AUTOSIZE );// Create a window for display.
    imshow(winName, frame);
    cv::waitKey(0);
}

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

void FrameStreamer::sendFrame(cv::Mat frame)
{
    struct FrameMessage msg;

    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf, {cv::IMWRITE_JPEG_QUALITY, 65});

    showImage(frame, "Before sending");

    strcpy(msg.name, name.c_str()); //TODO: check length
    memcpy(msg.data, buf.data(), buf.size());

    if (sendto(mySocket, &msg, sizeof(msg), 0, (const struct sockaddr *)&clientAddress, sizeof(clientAddress)) < 0)
    {
        close(mySocket);
        throw StreamException("Cannot send", errno);
    }
    else
    {
        std::cout << "sent image" << std::endl;
    }
}

void FrameStreamer::receiveFrame()
{
    struct FrameMessage msg;

    unsigned int len = sizeof(clientAddress);

    if (recvfrom(mySocket, &msg, sizeof(msg), MSG_WAITALL, (struct sockaddr *)&clientAddress, &len) < 0)
    {
        close(mySocket);
        throw StreamException("Cannot receive message", errno);
    }

    std::vector<unsigned char> buf(msg.data, msg.data + MAX_IMG_SIZE);
    cv::Mat frame = cv::imdecode(buf, cv::IMREAD_UNCHANGED);

    showImage(frame, "Received frame");
}

FrameStreamer::~FrameStreamer() { close(mySocket); }
