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

    strcpy(msg.data, name.c_str());
    msg.name_length = sizeof(name.c_str());
    msg.packet_id = 0;
    msg.frame_id = 0;
    msg.frames_in_packet = 1;

    std::vector<uchar> buf;
    int available_space = DATAGRAM_SIZE - msg.name_length - 4 * sizeof(short int) - 1;

    cv::imencode(".jpg", frame, buf, {cv::IMWRITE_JPEG_QUALITY, 65});

    showImage(frame, "Before sending");

    // std::cout << buf.size() << " " << available_space << std::endl;
    if (buf.size() <= available_space)
    {
        memcpy(msg.data + msg.name_length, buf.data(), buf.size());
        std::cout << "image copied\n";
    }
    else
    {
        std::cout << "Image is too big\n";
    }

    std::cout << "msg size " << sizeof(msg) << "\n";
    std::cout << name << " " << name.c_str() << " " << sizeof(name.c_str()) << " " << sizeof("test") << std::endl;

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
    unsigned int addr_size = sizeof(clientAddress);

    struct FrameMessage msg;

    if (recvfrom(mySocket, &msg, sizeof(msg), MSG_WAITALL, (struct sockaddr *)&clientAddress, &addr_size) < 0)
    {
        close(mySocket);
        throw StreamException("Cannot receive message", errno);
    }

    char *name = new char (msg.name_length);
    strcpy(name, msg.data);

    std::vector<unsigned char> buf(msg.data + msg.name_length, msg.data + DATAGRAM_SIZE - 4 * sizeof(short int) - 1);
    cv::Mat frame = cv::imdecode(buf, cv::IMREAD_UNCHANGED);

    showImage(frame, name);
}

FrameStreamer::~FrameStreamer() { close(mySocket); }
