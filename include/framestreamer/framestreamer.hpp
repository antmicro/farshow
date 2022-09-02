#pragma once

#include <arpa/inet.h>          //sockaddr_in
#include <opencv2/core/mat.hpp> // cv::Mat
#include <string>

#define DATAGRAM_SIZE 65507

/**
 * Message with the frame (or part of it)
 */
struct FrameMessage
{
    short int name_length;                                ///< length of stream name
    short int frame_id;                                   ///< id of the frame in this stream
    short int part_id;                                    ///< part id
    short int total_parts;                                ///< How many parts of the frame were send
    char data[DATAGRAM_SIZE - 4 * sizeof(short int) - 1]; ///< name of the stream (name_length bytes) and frame
                                                          ///< (remaining bytes)
};

/**
 * Streams the frames using UDP protocol
 */
class FrameStreamer
{
public:
    /**
     * Fill address structures, create socket and bind it.
     *
     * @param clientIp Client's ip address
     * @param clientPort Client's port
     * @param serverPort Server's port
     * @param serverIp Server's ip address (if not provided, binds the socket to all available interfaces)
     */
    FrameStreamer(std::string clientIp, int clientPort, int myPort = 1100, std::string myAddress = "");

    /*
     * Close the socket
     */
    ~FrameStreamer();

    /**
     * Encode the frame and send it (in parts if it's too big to fit the datagram).
     *
     * @param frame Frame to send
     */
    void sendFrame(cv::Mat frame);

    /**
     * Receive and display the frame
     */
    void receiveFrame();

    std::string stream_name; ///< name of the stream

private:
    struct sockaddr_in serverAddr; ///< Server's ip address structure
    struct sockaddr_in clientAddr; ///< Client's ip address structure
    int serverSocket;              ///< Socket binded to server address
};
