#include "framestreamer/framereceiver.hpp"
#include "framestreamer/streamexception.hpp"

#include <opencv2/imgcodecs.hpp>
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
        std::cout << "received part " << msg.header.part_id + 1 << "/" << msg.header.total_parts << " of frame "
                  << msg.header.frame_id << std::endl;
    }

    return msg;
}

cv::Mat FrameReceiver::prepareToShow(std::list<FrameContainer>::iterator frame)
{
    // delete previous, uncomplete frames
    streams[frame->name].erase(streams[frame->name].begin(), frame);

    // decode the frame
    return cv::imdecode((*frame).img, cv::IMREAD_UNCHANGED);
}

std::list<FrameContainer>::iterator FrameReceiver::addPart(FrameMessage msg)
{
    unsigned part_size = DATAGRAM_SIZE - sizeof(msg.header) - 3 - msg.header.name_length;

    // Get stream name
    std::unique_ptr<char> name = std::make_unique<char>(msg.header.name_length);
    strcpy(name.get(), msg.data);

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
    memcpy(itr->img.data + msg.header.part_id * part_size, msg.data + msg.header.name_length, part_size);
    itr->added_parts++;

    return itr;
}

Frame FrameReceiver::receiveFrame()
{
    FrameMessage frame_part;
    std::list<FrameContainer>::iterator frame;

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
