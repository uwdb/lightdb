#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "================"
echo "SIDE BY SIDE HEVC->HEVC"

echo "----------------"
echo "4K Input, side-by-side"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -y -loglevel error -i $file -filter:v "crop=in_w/2:in_h:0:0" -vcodec hevc_nvenc left.hevc
ffmpeg -hide_banner -y -loglevel error -i $file -filter:v "crop=in_w/2:in_h:in_w/2:0" -vcodec hevc_nvenc right.hevc
time ./side-by-side.sh left.hevc right.hevc out.hevc

echo "================"
echo "SIDE BY SIDE HEVC->HEVC"

echo "----------------"
echo "4K Input, self-overlay"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./self-overlay.sh $file out.hevc

rm left.hevc
rm right.hevc
rm out.hevc
