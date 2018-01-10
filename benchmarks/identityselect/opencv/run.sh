#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input, identity select"

width=3840
height=2048
left=0
top=0
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

rm input.mp4
