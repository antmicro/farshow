#pragma once
#include "framestreamer/udpinterface.hpp"

#include "opencv2/core/mat.hpp"
#include <GL/glew.h> //GLuint
#include <list>
#include <unordered_map>

/**
 * Frame ready to show
 */
typedef struct Frame
{
    std::string name;   ///< name of the stream
    cv::Mat img;        ///< image
    GLuint texture = 0; ///< image loaded as a texture
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
     * Tells if the frame has all parts.
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
 * Class for collecting frames from a stream.
 */
class FrameReceiver : public UdpInterface
{
public:
    /**
     * Fills client addres structure, creates a socket and binds it.
     *
     * @param client_port Client's port
     * @param client_address Client's ip address (if not provided, binds the socket to all available interfaces)
     */
    FrameReceiver(std::string client_address = "", int client_port = 1100);

    /**
     * Receives and displays the frame
     *
     * @returns Frame ready to display
     */
    Frame receiveFrame();

private:
    /**
     * Receives a message with a frame part
     *
     * @returns Received message with a frame part
     */
    FrameMessage receiveFramePart();

    /**
     * Deletes uncomplete frames before this frame, and decodes the frame
     *
     * @frame Iterator to the complete frame
     *
     * @returns Decoded frame
     */
    cv::Mat prepareToShow(std::list<FrameContainer>::iterator frame);

    /**
     * Delete uncomplete frames before this frame, decode the frame and create a texture with it
     *
     * @frame Iterator to the container with a complete image
     *
     * @returns Complete frame, ready to show
     */
    Frame putFrameTogether(std::list<FrameContainer>::iterator frame_container);

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
