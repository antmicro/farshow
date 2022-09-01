#include "framestreamer/framestreamer.hpp"

int main(int argc, char **argv)
{
    FrameStreamer streamer = FrameStreamer("127.0.0.1", 1101);

    streamer.receiveMessage();
    streamer.sendMessage("~server");
}
