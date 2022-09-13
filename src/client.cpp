#include "framestreamer/framereceiver.hpp"
#include "framestreamer/utils.hpp"

#include <opencv2/highgui.hpp>

/**
 * Client receives and shows streams
 */

int main(int argc, char **argv)
{
    FrameReceiver streamer = FrameReceiver("127.0.0.1");
    Frame frame;

    while (1)
    {
        frame = streamer.receiveFrame();
        showImage(frame.img, frame.name);
    }
}
