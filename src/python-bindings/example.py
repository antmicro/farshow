import farshow
import cv2
import signal
import argparse


def main():

    running = False

    parser = argparse.ArgumentParser(prog="Frame-streamer server",
                                     description=("A demo for frame-streamer – a minimalistic library to stream frames from e.g. "
                                                  "embedded devices.\nServer is capturing and streaming the frames."))
    parser.add_argument('-i', '--ip', required=True,
                       help="IP address of the client, which should receive stream. To send to multiple clients, enter broadcast address")
    parser.add_argument('-p', '--port', default=1100, type=int,
                        help="Port of the client, which will receive stream")
    parser.add_argument('-e', '--extension', default='jpg', choices=['jpg', 'png'],
                        help="Extension of the format in which frames will be send (e.g. `.jpg`, `.png`)")
    parser.add_argument('-q', '--quality', type=int,
                        help="Frames quality factor (for jpg in range 0 to 100, for png it's compression rate from 0 to 9)")
    parser.add_argument('-s', '--source', default='/dev/video0',
                        help="Filename of a camera device, which will be a stream source")

    args = parser.parse_args()

    img_types = {
            'jpg': ['.jpg', cv2.IMWRITE_JPEG_QUALITY, 65],
            'png': ['.png', cv2.IMWRITE_PNG_COMPRESSION, 5]
    }

    extension = img_types.get(args.extension)[0]
    encodingParams = img_types.get(args.extension)[1:3]
    if args.quality is not None:
        encodingParams[1] = args.quality

    originalHandler = signal.getsignal(signal.SIGINT)

    def signalHandler(signum, frame):
        print("Closing server...")
        global running
        running = False
        signal.signal(signal.SIGINT, originalHandler)

    signal.signal(signal.SIGINT, signalHandler)
    running = True
    cap = cv2.VideoCapture(args.source)
    streamer = farshow.FrameSender(args.ip, args.port)

    while running:
        ret, frame = cap.read()
        if not ret:
            break
        streamer.sendFrame(frame, "input", extension, encodingParams)

        frame = cv2.blur(frame, (7, 7))
        streamer.sendFrame(frame, "blur", extension, encodingParams)

        gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray_frame = cv2.adaptiveThreshold(gray_frame, 255,
                                           cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                           cv2.THRESH_BINARY, 11, 2)
        streamer.sendFrame(gray_frame, "threshold", extension, encodingParams)


if __name__ == "__main__":
    main()
