#pragma once
#include "framestreamer/udpinterface.hpp"

#include "opencv2/core/hal/interface.h"
#include "opencv2/core/mat.hpp"
#include <list>
#include <unordered_map>

/**
 * Frame ready to show
 */
typedef struct Frame
{
    std::string name; ///< name of the stream
    cv::Mat img;      ///< image
} Frame;

/**
 * Container for a received frame
 */
class FrameContainer
{
public:
    /**
     * Constructor
     *
     * @param id frame id
     * @param total_parts number of parts we're waiting for
     * @param name stream to which the frame belongs
     * @param frame_size Total size of the image
     */
    FrameContainer(unsigned id, unsigned total_parts, std::string name, unsigned frame_size)
        : id(id), total_parts(total_parts), added_parts(0), name(name)
    {
        img = cv::Mat(1, frame_size, CV_8UC1);
    }

    /**
     * If the frame has all parts
     *
     * @returns True if the frame has all parts, false otherwise
     */
    bool isComplete() { return total_parts == added_parts; }

    unsigned id;          ///< frame id
    unsigned total_parts; ///< number of parts which we're waiting for
    unsigned added_parts; ///< number of received parts
    cv::Mat img;          ///< the frame data
    std::string name;     ///< stream to which the frame belongs
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
     * @param server_address Server's ip address (if not provided, binds the socket to all available interfaces)
     */
    FrameReceiver(std::string server_address = "", int server_port = 1100);

    /**
     * Receive and display the frame
     *
     * @returns Frame ready to display
     */
    Frame receiveFrame();

private:
    /**
     * Receive a message with a frame part
     *
     * @returns Received message with a frame part
     */
    FrameMessage receiveFramePart();

    /**
     * Delete uncomplete frames before this frame, and decode the frame
     *
     * @frame Iterator to the complete frame
     *
     * @returns Decoded frame
     */
    cv::Mat prepareToShow(std::list<FrameContainer>::iterator frame);

    /**
     * Assign the frame part to a proper frame in a proper stream
     *
     * @param frame_part Frame part to add
     *
     * @returns Iterator to the place there the frame was added
     */
    std::list<FrameContainer>::iterator addPart(FrameMessage frame_part);

    std::unordered_map<std::string, std::list<FrameContainer>> streams; ///< All available streams mapped to their
                                                                        ///< uncomplete frames. The frames are sorted by
                                                                        ///< id.
};
