#!/usr/bin/env bash

ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $1 -vf format=gray -vcodec hevc_nvenc $2
