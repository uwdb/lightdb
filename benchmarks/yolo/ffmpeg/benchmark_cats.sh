cmake .
make
time ./ffmpeg_tiler ../../datasets/cats/cats.h264 > /dev/null
