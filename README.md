# Displaying an RTMP stream with FFmpeg and SDL libraries

## Running

1. Clone repo with submodules: `git clone --recursive https://github.com/diftucs/ffmpeg-rtmp-display`.
2. `cd ffmpeg-rtmp-display/`.
3. Compile FFmpeg and this project: `./compile.sh`.
4. Run with local libraries: `LD_LIBRARY_PATH="ffmpeg/libavcodec:ffmpeg/libavdevice:ffmpeg/libavformat:ffmpeg/libavutil:ffmpeg/libswscale:SDL/build/.libs:$LD_LIBRARY_PATH" ./main`.

## Debugging

`./compile.sh` already compiles FFmpeg, SDL and this project with debug symbols. Run `LD_LIBRARY_PATH="ffmpeg/libavcodec:ffmpeg/libavdevice:ffmpeg/libavformat:ffmpeg/libavutil:ffmpeg/libswscale:SDL/build/.libs:$LD_LIBRARY_PATH" gdb ./main` to initiate debugging.

## Important notes

- This project is written to work with https://github.com/diftucs/ffmpeg-rtmp-screensharing
- The stream is read from `rtmp://localhost/publishlive/livestream`
