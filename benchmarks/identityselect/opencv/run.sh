#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "1K Input"

width=960
height=512
left=0
top=0
file=$DATASET_PATH${DATASET_NAME}1K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "----------------"
echo "2K Input"

width=1920
height=1024
left=0
top=0
file=$DATASET_PATH${DATASET_NAME}2K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "----------------"
echo "4K Input"

width=3840
height=2048
left=0
top=0
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

rm input.mp4
