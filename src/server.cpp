#include "framestreamer/framereceiver.hpp"

int main(int argc, char **argv)
{
    FrameReceiver streamer = FrameReceiver("127.0.0.1", 1101);

    streamer.receiveFrame();
}
