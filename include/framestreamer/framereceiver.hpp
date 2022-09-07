#pragma once
#include "framestreamer/udpinterface.hpp"
#include <unordered_map>
#include <forward_list>
#include <iostream>


/**
 * Container for a received frame
 */
class Frame
{
 public:

    Frame(){} // TODO: delete

    /**
     * Constructor
     */
    Frame(unsigned id, unsigned total_parts, std::string name, unsigned frame_size): id(id), total_parts(total_parts), added_parts(0), name(name)
    {
        img = cv::Mat(1, frame_size, CV_8UC1);
    }

    /**
     * If the frame has all parts
     */
    bool isComplete() {return total_parts == added_parts;}

    unsigned id; ///< frame id
    unsigned total_parts; ///< number of parts which we're waiting for
    unsigned added_parts; ///< number of received parts
    cv::Mat img; ///< the frame data
    std::string name; ///< stream to which the frame belongs
};


/**
 * Receives frames from various streams, handles them and displays
 */
class FrameReceiver : public UdpInterface
{
public:
    /**
     * Fill server addres structure, create a socket and bind it.
     *
     * @param server_port Server's port
     * @param server_ip Server's ip address (if not provided, binds the socket to all available interfaces)
     */
    FrameReceiver(std::string server_address = "", int server_port = 1100);

    /**
     * Receive and display the frame
     */
    Frame receiveFrame();

private:
    /**
     * Receive a message with a frame part
     */
    FrameMessage receiveFramePart();

    /**
     * Assign the frame part to a proper frame in a proper stream
     *
     * @param frame_part Frame part to add
     */
    void addPart(FrameMessage frame_part);

    std::unordered_map<std::string, std::forward_list<Frame>> streams; ///< All available streams mapped to their frames. The frames are sorted by id.
    // TODO: store only uncompleted frames
};
