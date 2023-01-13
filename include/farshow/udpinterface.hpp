#pragma once

#include <arpa/inet.h> // sockaddr_in
#include <string>

#define DATAGRAM_SIZE 65507

namespace farshow
{

/**
 * Frame metadata
 */
struct FrameHeader
{
    unsigned name_length; ///< length of stream name
    unsigned frame_id;    ///< id of the frame in this stream
    unsigned part_id;     ///< part id
    unsigned total_parts; ///< how many parts of the frame were send
};

/**
 * Message with the frame (or part of it)
 */
typedef struct FrameMessage
{
    struct FrameHeader header;                     ///< metadata
    char data[DATAGRAM_SIZE - sizeof(header) - 3]; ///< name of the stream (name_length bytes) and frame
                                                   ///< (remaining bytes)
} FrameMessage;

/**
 * Handles UDP Interface
 */
class UdpInterface
{
public:
    /**
     * Fills client address structure and create a socket
     *
     * @param client_port Client's port
     * @param client_address Client's ip address (if not provided, binds the socket to all available interfaces)
     */
    UdpInterface(std::string client_address = "", int client_port = 1100);

    /*
     * Closes the socket
     */
    ~UdpInterface();

protected:
    struct sockaddr_in clientAddr = {0}; ///< client's ip address structure
    int mySocket;                        ///< socket binded to client address
};

}; // namespace farshow
