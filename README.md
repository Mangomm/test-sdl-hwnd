# test-sdl-hwnd demo

This project is based on https://github.com/libsdl-org/SDL/blob/main/test/testffmpeg.c.
The purpose is to test the implementation of hardware decoding + 0 copy rendering using a handle.

# Configuration
1. You need to change the following defines in testffmpeg.h:

static const char *file = "/home/admin1/xxx/test-sdl-hwnd/testvideo/video-ios.h264";

2. You may to change the following defines in test-sdl-hwnd.pro:
Modify the opengl dynamic library location.


# Run And Stop
At present, it is just a simple use case, after the program runs, click the button to control play and stop.

# Preriquisitorites

* The latest sdl3 is required
* Support H264 hardware decoding/encoding
* Linux based operating system
* qt creator

# Libraries used

* sdl <https://github.com/libsdl-org/SDL>
* ffmpeg <https://ffmpeg.org>
