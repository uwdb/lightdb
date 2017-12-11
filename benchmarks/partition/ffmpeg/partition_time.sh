#!/usr/bin/env bash

input=$1
t=$2

ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $input \
  -vcodec hevc_nvenc -r 30 -g 15 \
  -f segment -segment_time $t -segment_time_delta 0.05 -segment_format mp4 output-%d.mp4
