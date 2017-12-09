dataset=$1
ffmpeg -y -i ../../datasets/${dataset}/${dataset}4K.h264 -c copy input.mp4
time ./opencv_tiler input.mp4
du -ch out*.mp4 | tail -n 1
rm out*.mp4
