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
    msg.frame_id = 0;
    msg.part_id = 0;

    std::vector<uchar> buf;
    int available_space = DATAGRAM_SIZE - msg.name_length - 4 * sizeof(short int) - 1;

    cv::imencode(".jpg", frame, buf, {cv::IMWRITE_JPEG_QUALITY, 65});

    showImage(frame, "Before sending");

    if (buf.size() <= available_space)
    {
        msg.parts_in_packet = 1;
    }
    else
    {
        msg.parts_in_packet = std::ceil((float)buf.size()/available_space); //TODO: don't send name in further messages
        std::cout << buf.size() << "/ " << available_space << " = " << msg.parts_in_packet << std::endl; 
    }

    std::cout << "msg size " << sizeof(msg) << "\n";
    std::cout << name << " " << name.c_str() << " " << sizeof(name.c_str()) << " " << sizeof("test") << std::endl;

    // Send packets
    uchar *data = buf.data();

    while (data <= buf.data() + buf.size())
    {
        memcpy(msg.data + msg.name_length, data, available_space);
        if (sendto(mySocket, &msg, sizeof(msg), 0, (const struct sockaddr *)&clientAddress, sizeof(clientAddress)) < 0)
        {
            close(mySocket);
            throw StreamException("Cannot send", errno);
        }
        else
        {
            std::cout << "sent image" << std::endl;
        }
        msg.part_id++;
        data += available_space;
    }
}

void FrameStreamer::receiveFrame()
{
    unsigned int addr_size = sizeof(clientAddress);

    struct FrameMessage msg;

    if (recvfrom(mySocket, &msg, sizeof(msg), MSG_WAITALL | MSG_PEEK, (struct sockaddr *)&clientAddress, &addr_size) < 0)
    {
        close(mySocket);
        throw StreamException("Cannot receive message", errno);
    }

    char *name = new char (msg.name_length);
    strcpy(name, msg.data);

    int part_size = DATAGRAM_SIZE - 4 * sizeof(short int) - 1 - msg.name_length;
    std::vector<unsigned char> buf(msg.parts_in_packet * part_size);
    unsigned char *position = buf.data();

    std::cout << "Prepared for " << msg.parts_in_packet << "parts\n";

    for (int i=0; i < msg.parts_in_packet; i++)
    {
        if (recvfrom(mySocket, &msg, sizeof(msg), MSG_WAITALL, (struct sockaddr *)&clientAddress, &addr_size) < 0)
        {
            close(mySocket);
            throw StreamException("Cannot receive message", errno);
        }
        else
        {
            std::cout << "received part\n";
        }
        memcpy(position, msg.data + msg.name_length, part_size);
        position += part_size;
    }

    // std::vector<unsigned char> buf(msg.data + msg.name_length, msg.data + DATAGRAM_SIZE - 4 * sizeof(short int) - 1);
    cv::Mat frame = cv::imdecode(buf, cv::IMREAD_UNCHANGED);

    showImage(frame, name);

    delete(name);
}

FrameStreamer::~FrameStreamer() { close(mySocket); }
