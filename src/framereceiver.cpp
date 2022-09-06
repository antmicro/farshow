#include "framestreamer/framereceiver.hpp"
#include "framestreamer/streamexception.hpp"
#include "framestreamer/utils.hpp"

#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

void FrameReceiver::receiveFrame()
{
    unsigned int addr_size = sizeof(clientAddr);

    FrameMessage msg;

    // Check stream name and number of parts
    if (recvfrom(serverSocket, &msg, sizeof(msg), MSG_WAITALL | MSG_PEEK, (struct sockaddr *)&clientAddr, &addr_size) < 0)
    {
        close(serverSocket);
        throw StreamException("Cannot receive message", errno);
    }

    std::unique_ptr<char> name = std::make_unique<char>(msg.header.name_length);
    strcpy(name.get(), msg.data);

    // Wait for data
    int part_size = DATAGRAM_SIZE - sizeof(FrameHeader) - 1 - msg.header.name_length;
    std::vector<unsigned char> compressed_frame(msg.header.total_parts * part_size);
    unsigned char *position = compressed_frame.data();

    for (int i = 0; i < msg.header.total_parts; i++)
    {
        if (recvfrom(serverSocket, &msg, sizeof(msg), MSG_WAITALL, (struct sockaddr *)&clientAddr, &addr_size) < 0)
        {
            close(serverSocket);
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
    cv::Mat frame = cv::imdecode(compressed_frame, cv::IMREAD_UNCHANGED);
    showImage(frame, name.get());
}
