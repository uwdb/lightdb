#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input, overlay"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -filter:v "crop=in_w/2:in_h:0:0" input.mp4
time ./opencv_union overlay input.mp4 input.mp4

echo "----------------"
echo "4K Input, stack"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

#ffmpeg -hide_banner -loglevel error -y -i $file -filter:v "crop=in_w/2:in_h:0:0" input.mp4
#time ./opencv_union stack input.mp4

#rm input.mp4
