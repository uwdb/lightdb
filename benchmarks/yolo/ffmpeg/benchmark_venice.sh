cmake .
make
time ./ffmpeg_tiler ../../datasets/venice/venic4K.h264 > /dev/null
