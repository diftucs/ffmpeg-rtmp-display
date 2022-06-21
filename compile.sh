(
    echo "Building FFmpeg" && \
    cd ffmpeg/ && \
    ./configure --enable-debug --enable-gpl --enable-libx264 --enable-shared && \
    make
)

(
    echo "Building SDL" && \
    cd SDL/ && \
    ./configure && \
    make
)

echo
echo "Building main.cpp"
g++ main.cpp -o main -g -Iffmpeg/libavcodec -Iffmpeg/libavformat -Iffmpeg/libavutil -Iffmpeg/libswscale -ISDL/include -Lffmpeg/libavcodec -Lffmpeg/libavformat -Lffmpeg/libavutil -Lffmpeg/libswscale -LSDL2/build/.libs -lavcodec -lavformat -lavutil -lswscale -lSDL2
