#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "1K Input"

width=384
height=384
left=128
top=128
file=$DATASET_PATH${DATASET_NAME}1K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_map input.mp4

echo "----------------"
echo "2K Input"

width=768
height=768
left=256
top=256
file=$DATASET_PATH${DATASET_NAME}2K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_map input.mp4

echo "----------------"
echo "4K Input"

width=1440
height=1440
left=480
top=480
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_map input.mp4

rm input.mp4
