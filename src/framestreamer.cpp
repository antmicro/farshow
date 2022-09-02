#include "framestreamer/framestreamer.hpp"
#include "framestreamer/streamexception.hpp"

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

/**
 * Display a window with the image)
 */
void showImage(cv::Mat frame, std::string winName)
{
    namedWindow(winName, cv::WINDOW_AUTOSIZE); // Create a window
    imshow(winName, frame);
    cv::waitKey(0);
}

FrameStreamer::FrameStreamer(std::string clientIp, int clientPort, int serverPort, std::string serverIp)
{
    // Create address structures
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = (serverIp == "" ? htonl(INADDR_ANY) : inet_addr(serverIp.c_str()));

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(clientPort);
    clientAddr.sin_addr.s_addr = inet_addr(clientIp.c_str());

    // Create and bind the socket
    serverSocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1)
    {
        throw StreamException("Cannot create socket", errno);
    }

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        close(serverSocket);
        throw StreamException("Cannot bind", errno);
    }
}

void FrameStreamer::sendFrame(cv::Mat frame)
{
    // Create a message
    struct FrameMessage msg;

    strcpy(msg.data, stream_name.c_str());
    msg.name_length = sizeof(stream_name.c_str());
    msg.frame_id = 0;
    msg.part_id = 0;

    // Compress the image
    std::vector<uchar> compressed_frame;
    int available_space = DATAGRAM_SIZE - msg.name_length - 4 * sizeof(short int) - 1;

    cv::imencode(".jpg", frame, compressed_frame, {cv::IMWRITE_JPEG_QUALITY, 95});

    showImage(frame, "Before sending");

    if (compressed_frame.size() <= available_space)
    {
        // Will fit one message
        msg.total_parts = 1;
    }
    else
    {
        // Split frame to parts
        msg.total_parts = std::ceil((float)compressed_frame.size() / available_space);
    }

    // Send parts
    uchar *position = compressed_frame.data(); // Data not send yet

    while (position <= compressed_frame.data() + compressed_frame.size())
    {
        memcpy(msg.data + msg.name_length, position, available_space);
        if (sendto(serverSocket, &msg, sizeof(msg), 0, (const struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0)
        {
            close(serverSocket);
            throw StreamException("Cannot send", errno);
        }
        else
        {
            std::cout << "Sent part " << msg.part_id + 1 << "/" << msg.total_parts << std::endl;
        }
        msg.part_id++;
        position += available_space;
    }
}

void FrameStreamer::receiveFrame()
{
    unsigned int addr_size = sizeof(clientAddr);

    struct FrameMessage msg;

    // Check stream name and number of parts
    if (recvfrom(serverSocket, &msg, sizeof(msg), MSG_WAITALL | MSG_PEEK, (struct sockaddr *)&clientAddr, &addr_size) < 0)
    {
        close(serverSocket);
        throw StreamException("Cannot receive message", errno);
    }

    std::unique_ptr<char> name = std::make_unique<char>(msg.name_length);
    strcpy(name.get(), msg.data);

    // Wait for data
    int part_size = DATAGRAM_SIZE - 4 * sizeof(short int) - 1 - msg.name_length;
    std::vector<unsigned char> compressed_frame(msg.total_parts * part_size);
    unsigned char *position = compressed_frame.data();

    for (int i = 0; i < msg.total_parts; i++)
    {
        if (recvfrom(serverSocket, &msg, sizeof(msg), MSG_WAITALL, (struct sockaddr *)&clientAddr, &addr_size) < 0)
        {
            close(serverSocket);
            throw StreamException("Cannot receive message", errno);
        }
        else
        {
            std::cout << "received part " << msg.part_id + 1 << "/" << msg.total_parts << std::endl;
        }
        memcpy(position, msg.data + msg.name_length, part_size);
        position += part_size;
    }

    // Show the frame
    cv::Mat frame = cv::imdecode(compressed_frame, cv::IMREAD_UNCHANGED);
    showImage(frame, name.get());
}

FrameStreamer::~FrameStreamer() { close(serverSocket); }
