#include "framestreamer/framereceiver.hpp"
#include "framestreamer/streamexception.hpp"
#include "framestreamer/utils.hpp"

#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

void printList(std::forward_list<FrameContainer> &stream)
{
     std::cout << stream.begin()->name << ": ";

     for (auto const &i: stream)
     {
         std::cout << i.id << " ";
     }
     std::cout << std::endl;
}



FrameReceiver::FrameReceiver(std::string server_address, int server_port) : UdpInterface(server_address, server_port)
{
    if (bind(mySocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        close(mySocket);
        throw StreamException("Cannot bind", errno);
    }
}


FrameMessage FrameReceiver::receiveFramePart()
{
    FrameMessage msg;

    // Wait for data
    if (recv(mySocket, &msg, sizeof(msg), 0) < 0)
    {
        close(mySocket);
        throw StreamException("Cannot receive message", errno);
    }
    else
    {
        std::cout << "received part " << msg.header.part_id + 1 << "/" << msg.header.total_parts << std::endl;
    }

    return msg;
}

cv::Mat FrameReceiver::prepareToShow(std::forward_list<FrameContainer>::iterator frame)
{
    // delete previous, uncomplete frames
    streams[frame->name].erase_after(streams[frame->name].before_begin(), frame);

    // decode the frame
    return cv::imdecode((*frame).img, cv::IMREAD_UNCHANGED);
}

std::forward_list<FrameContainer>::iterator FrameReceiver::addPart(FrameMessage msg)
{
    unsigned part_size = DATAGRAM_SIZE - sizeof(FrameHeader) - 3 - msg.header.name_length;

    // Get stream name
    std::unique_ptr<char> name = std::make_unique<char>(msg.header.name_length);
    strcpy(name.get(), msg.data);

    // Find a frame
    auto itr = streams[name.get()].begin();
    auto prev = itr;

    if (!streams[name.get()].empty())
    {
        while (itr != streams[name.get()].end() && itr->id < msg.header.frame_id)
        {
            prev = itr;
            itr++;
        }
    }
    if(itr == streams[name.get()].end() || itr->id > msg.header.frame_id)
    {
        std::cout << "New frame\n";
        // Create a new frame
        unsigned frame_size = msg.header.total_parts * part_size;
        FrameContainer frame = FrameContainer(msg.header.frame_id, msg.header.total_parts, name.get(), frame_size);
        if (itr == streams[name.get()].begin())
        {
            streams[name.get()].emplace_front(frame);
            itr = streams[name.get()].begin();
        }
        else
        {
            itr = streams[name.get()].insert_after(prev, frame);
        }
    }
    printList(streams[name.get()]);

    // Copy image data to the frame pointed by iterator
    memcpy(itr->img.data + msg.header.part_id * part_size, msg.data + msg.header.name_length, part_size);
    itr->added_parts++;

    return itr;
}

Frame FrameReceiver::receiveFrame()
{
    FrameMessage frame_part;
    std::forward_list<FrameContainer>::iterator frame;

    while (1)
    {
        FrameMessage frame_part = receiveFramePart();
        auto frame = addPart(frame_part);

        if ((*frame).isComplete())
        {
            Frame res;
            res.img = prepareToShow(frame);
            res.name = frame->name;
            return res;
        }
    }
}
