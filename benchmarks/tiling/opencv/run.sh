ffmpeg -y -i $1 -c copy input.mp4
time ./opencv_tiler input.mp4
du -ch out*.mp4 | tail -n 1
rm out*.mp4
