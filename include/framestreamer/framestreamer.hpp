#pragma once

#include <arpa/inet.h> //sockaddr_in
#include <opencv2/core/mat.hpp> // cv::Mat
#include <string>

#define MAX_STREAM_NAME_LEN 128
#define MAX_IMG_SIZE 65507 - MAX_STREAM_NAME_LEN // because of udp datagram size

struct FrameMessage
{
    char name[MAX_STREAM_NAME_LEN];
    unsigned char data[MAX_IMG_SIZE];
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

    std::string name;

private:
    struct sockaddr_in myAddress;
    struct sockaddr_in clientAddress;
    int mySocket;
};
