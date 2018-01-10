#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

echo Executing map YOLO benchmark
ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_map input.mp4

rm input.mp4
