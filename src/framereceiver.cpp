#include "framestreamer/framereceiver.hpp"
#include "framestreamer/streamexception.hpp"
#include "framestreamer/utils.hpp"
#include "imgui.h"

#include <unistd.h>

void printList(std::list<FrameContainer> &stream)
{
    std::cout << stream.begin()->name << ": ";

    for (auto const &i : stream)
    {
        std::cout << i.id << " ";
    }
    std::cout << std::endl;
}

FrameReceiver::FrameReceiver(std::string client_address, int client_port) : UdpInterface(client_address, client_port)
{
    if (bind(mySocket, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) == -1)
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
        std::cout << "received part " << msg.header.part_id + 1 << "/" << msg.header.total_parts << " of frame "
                  << msg.header.frame_id;
    }

    return msg;
}

std::list<FrameContainer>::iterator FrameReceiver::addPart(FrameMessage msg)
{
    unsigned part_size = DATAGRAM_SIZE - sizeof(msg.header) - 3 - msg.header.name_length;

    // Get stream name
    std::unique_ptr<char> name = std::make_unique<char>(msg.header.name_length);
    strcpy(name.get(), msg.data);
    std::cout << " (" << name << ")" << std::endl;

    // Find a frame
    auto itr = streams[name.get()].begin();

    if (!streams[name.get()].empty())
    {
        while (itr != streams[name.get()].end() && itr->id < msg.header.frame_id)
        {
            itr++;
        }

        // Check id overflow
        // When the frame 0 comes after 4294967295, we don't want to place it at the beginning, but in the end, to start
        // a new lap. To do this we check, whether there's a "large gap" between new frame's id and its successor's id.
        // If there is, we add the frame in the end (after the frames with very large id's). Because we use UDP, frames'
        // parts can be late. I've set the size of the "gap" to UINT_MAX/4, to minimize the probability of misplacing
        // the frame, but this value can be changed later if we'll find a better one.
        if (itr != streams[name.get()].end() && itr->id - msg.header.frame_id > UINT_MAX / 4)
        {
            // Look for the proper place from the end
            itr = streams[name.get()].end();
            while (std::prev(itr)->id < UINT_MAX * 3 / 4 && std::prev(itr)->id >= msg.header.frame_id)
            {
                itr--;
            }
        }
    }
    if (itr == streams[name.get()].end() || itr->id > msg.header.frame_id)
    {
        // Create a new frame
        unsigned frame_size = msg.header.total_parts * part_size;
        FrameContainer frame = FrameContainer(msg.header.frame_id, msg.header.total_parts, name.get(), frame_size);
        itr = streams[name.get()].insert(itr, frame);
    }
    // Copy image data to the frame pointed by iterator
    memcpy(itr->img.data() + msg.header.part_id * part_size, msg.data + msg.header.name_length, part_size);
    itr->added_parts++;

    return itr;
}

cv::Mat FrameReceiver::prepareToShow(std::list<FrameContainer>::iterator frame)
{
    // delete previous, uncomplete frames
    streams[frame->name].erase(streams[frame->name].begin(), frame);

    // decode the frame
    return cv::imdecode((*frame).img, cv::IMREAD_UNCHANGED);
}

Frame FrameReceiver::putFrameTogether(std::list<FrameContainer>::iterator frame_container)
{
    Frame frame;
    frame.img = prepareToShow(frame_container);
    frame.name = frame_container->name;

    if(frame.img.channels() == 1)
    {
        cv::cvtColor(frame.img, frame.img, cv::COLOR_GRAY2RGB);
    }
    frame.texture = loadTextureFromCVMat(frame.img);
    IM_ASSERT(frame.texture);

    return frame;
}

Frame FrameReceiver::receiveFrame()
{
    FrameMessage frame_part;
    std::list<FrameContainer>::iterator frame;

    while (1)
    {
        FrameMessage frame_part = receiveFramePart();
        frame = addPart(frame_part);

        if ((*frame).isComplete())
        {
            return putFrameTogether(frame);
        }
    }
}
