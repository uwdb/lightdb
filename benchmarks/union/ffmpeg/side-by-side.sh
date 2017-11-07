#!/usr/bin/env bash

ffmpeg -hide_banner -y -loglevel error -vcodec hevc_cuvid -i $1 -i $1 -filter_complex hstack -vcodec hevc_nvenc $2
