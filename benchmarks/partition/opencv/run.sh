#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input, 1.5s time partition"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION
time=1.5

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_partitioner time input.mp4 $time

echo "----------------"
echo "4K Input, theta partition"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_partitioner tile input.mp4 1 4

echo "----------------"
echo "4K Input, phi partition"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_partitioner tile input.mp4 4 1

rm input.mp4
rm tile*.mp4
