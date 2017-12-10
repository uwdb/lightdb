#!/usr/bin/env bash

input=$1
width=$2
height=$3
file=$4

ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $input -filter_complex \
  "[0:v]crop=$width:$height/4:0:0[out1];
   [0:v]crop=$width:$height/4:0:1*$height/4[out2];
   [0:v]crop=$width:$height/4:0:2*$height/4[out3];
   [0:v]crop=$width:$height/4:0:3*$height/4[out4]" \
  -map [out1] -vcodec hevc_nvenc 1$file \
  -map [out2] -vcodec hevc_nvenc 2$file \
  -map [out3] -vcodec hevc_nvenc 3$file \
  -map [out4] -vcodec hevc_nvenc 4$file
