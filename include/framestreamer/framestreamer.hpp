#pragma once

#include <arpa/inet.h> //sockaddr_in
#include <opencv2/core/mat.hpp> // cv::Mat
#include <string>

#define DATAGRAM_SIZE 65507

struct FrameMessage
{
    short int name_length;
    short int frame_id;
    short int part_id;
    short int parts_in_packet;
    char data[DATAGRAM_SIZE - 4 * sizeof(short int) - 1];
};

class FrameStreamer
{
public:
    /**
     * Fill address structures, create socket and bind it.
     */
    FrameStreamer(std::string clientIp, int clientPort, int myPort = 1100, std::string myAddress = "");

    /*
     * Close the socket
     */
    ~FrameStreamer();

    void sendMessage(std::string text);
    void receiveMessage(); //TODO: delete
    void sendFrame(cv::Mat frame);
    void receiveFrame(); // TODO: delete

    std::string name; //QUESTION: Why isn't sizeof(name.c_str()) == sizeof("test") (8 vs 5)

private:
    struct sockaddr_in myAddress;
    struct sockaddr_in clientAddress;
    int mySocket;
};
