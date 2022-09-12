#pragma once

#include <arpa/inet.h> // sockaddr_in
#include <string>

#define DATAGRAM_SIZE 65507

/**
 * Frame metadata
 */
struct FrameHeader
{
    unsigned name_length; ///< length of stream name
    unsigned frame_id;    ///< id of the frame in this stream
    unsigned part_id;     ///< part id
    unsigned total_parts; ///< How many parts of the frame were send
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
     * Fill server address structure and create a socket
     *
     * @param server_port Server's port
     * @param server_address Server's ip address (if not provided, binds the socket to all available interfaces)
     */
    UdpInterface(std::string server_address = "", int server_port = 1100);

    /*
     * Close the socket
     */
    ~UdpInterface();

protected:
    struct sockaddr_in serverAddr; ///< Server's ip address structure
    int mySocket;                  ///< Socket binded to server address
};
