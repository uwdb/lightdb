#!/usr/bin/env bash

ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $1 -filter:v "crop=$2:$3:$4:$5" -vcodec hevc_nvenc $6
