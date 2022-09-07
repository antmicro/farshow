#include "framestreamer/framesender.hpp"
#include "framestreamer/streamexception.hpp"

#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

void FrameSender::sendFrame(cv::Mat frame, std::string extension, std::vector<int> encoding_params)
{
    // Create a message
    FrameMessage msg;

    strcpy(msg.data, stream_name.c_str());
    msg.header.name_length = sizeof(stream_name.c_str());
    msg.header.frame_id = curr_frame_id++;
    msg.header.part_id = 0;

    // Compress the image
    std::vector<uchar> compressed_frame;
    unsigned available_space = DATAGRAM_SIZE - msg.header.name_length - sizeof(FrameHeader) - 3;

    cv::imencode(extension, frame, compressed_frame, encoding_params);

    if (compressed_frame.size() <= available_space)
    {
        // Will fit one message
        msg.header.total_parts = 1;
    }
    else
    {
        // Split frame to parts
        msg.header.total_parts = std::ceil((float)compressed_frame.size() / available_space);
    }

    // Send parts
    uchar *position = compressed_frame.data(); // Data not send yet

    while (position <= compressed_frame.data() + compressed_frame.size())
    {
        memcpy(msg.data + msg.header.name_length, position, available_space);
        if (sendto(mySocket, &msg, sizeof(msg), 0, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            close(mySocket);
            throw StreamException("Cannot send", errno);
        }
        else
        {
            std::cout << "Sent part " << msg.header.part_id + 1 << "/" << msg.header.total_parts << std::endl;
        }
        msg.header.part_id++;
        position += available_space;
        usleep (frame_parts_delay);
    }
}
