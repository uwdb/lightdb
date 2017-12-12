#!/usr/bin/env bash

file=$1
col=$2
row=$3
rows=$4
cols=$5
out=$6

ffmpeg -hide_banner -y -loglevel error -vcodec hevc_cuvid -i $file -filter:v "crop=in_w/$cols:in_h/$rows:$col*in_w/$cols:$row*in_h/$rows" -vcodec hevc_nvenc $out
