#!/usr/bin/env bash

input=$1
start=$2
end=$3
out=$4
ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $input -ss 00:00:${start} -t 00:00:${end} -vcodec hevc_nvenc $out
