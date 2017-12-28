cmake .
make
time ./ffmpeg_tiler ../../datasets/coaster/timelapse4K.h264 > /dev/null
