#!/usr/bin/env bash

ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $1 -vf "gblur" -vcodec hevc_nvenc $2
