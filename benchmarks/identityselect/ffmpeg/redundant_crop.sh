#!/usr/bin/env bash

ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $1 -filter:v "[0:v]crop=$2:$3:$4:$5[c],[c]crop=$2/2:$3/2:0:0" -vcodec hevc_nvenc $6
