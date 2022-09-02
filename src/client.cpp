#include <framestreamer/framestreamer.hpp>

int main()
{
    FrameStreamer streamer = FrameStreamer("127.0.0.1", 1100, 1101, "127.0.0.1");
    streamer.receiveFrame();
    return 0;
}
