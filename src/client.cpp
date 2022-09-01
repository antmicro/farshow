#include <framestreamer/framestreamer.hpp>

int main()
{
    // create socket
    FrameStreamer streamer = FrameStreamer("127.0.0.1", 1100, 1101, "127.0.0.1");

    // streamer.sendMessage("~client");
    // streamer.receiveMessage();

    streamer.receiveFrame();
    return 0;
}
