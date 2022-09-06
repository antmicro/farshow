#include "framestreamer/framereceiver.hpp"

/**
 * Server receives and shows streams
 */

int main(int argc, char **argv)
{
    FrameReceiver streamer = FrameReceiver("127.0.0.1");

    streamer.receiveFrame();
}
