#!/usr/bin/env bash

input=$1
segment_count=$2
segment_time=$3

ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $input \
  -vcodec h264_nvenc -r 30 -g 30 \
  -f segment -segment_time $segment_time -segment_time_delta 0.05 -segment_format mp4 segment%d.mp4

for i in `seq 1 $segment_count`;
        do
                ffmpeg -hide_banner -y -loglevel error -i segment$i.mp4 -c copy segment$i.h264
        done
rm segment*.mp4

python create-time-concat.py $segment_count > script.sh
chmod +x script.sh
time ./script.sh
rm script.sh
