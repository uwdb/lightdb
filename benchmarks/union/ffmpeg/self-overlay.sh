#!/usr/bin/env bash

ffmpeg -y -hide_banner -loglevel error -i $1 -i $1 -filter_complex "[0:v][1:v]blend=all_mode='overlay'" -vcodec hevc_nvenc $2
