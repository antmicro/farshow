#include "framestreamer/framereceiver.hpp"
#include "framestreamer/streamexception.hpp"
#include "framestreamer/utils.hpp"

#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

FrameReceiver::FrameReceiver(std::string server_address, int server_port) : UdpInterface(server_address, server_port)
{
    if (bind(mySocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        close(mySocket);
        throw StreamException("Cannot bind", errno);
    }
}

void FrameReceiver::receiveFrame()
{
    FrameMessage msg;

    // Check stream name and number of parts
    if (recv(mySocket, &msg, sizeof(msg), MSG_WAITALL | MSG_PEEK) < 0)
    {
        close(mySocket);
        throw StreamException("Cannot receive message", errno);
    }

    std::unique_ptr<char> name = std::make_unique<char>(msg.header.name_length);
    strcpy(name.get(), msg.data);

    // Wait for data
    unsigned part_size = DATAGRAM_SIZE - sizeof(FrameHeader) - 3 - msg.header.name_length;

    unsigned frame_size = msg.header.total_parts * part_size;
    unsigned char *compressed_frame = new unsigned char[frame_size];
    unsigned char *position = compressed_frame;

    for (int i = 0; i < msg.header.total_parts; i++)
    {
        std::cout << "waiting for msg\n";
        if (recv(mySocket, &msg, sizeof(msg), 0) < 0)
        {
            close(mySocket);
            throw StreamException("Cannot receive message", errno);
        }
        else
        {
            std::cout << "received part " << msg.header.part_id + 1 << "/" << msg.header.total_parts << std::endl;
        }
        memcpy(position, msg.data + msg.header.name_length, part_size);
        position += part_size;
    }

    // Show the frame
    cv::Mat frame = cv::imdecode(cv::Mat(1, frame_size, CV_8UC1, compressed_frame), cv::IMREAD_UNCHANGED);
    showImage(frame, name.get());
}
