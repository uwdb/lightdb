#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "1K Input"

file=$DATASET_PATH${DATASET_NAME}1K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_partitioner input.mp4

echo "----------------"
echo "2K Input"

file=$DATASET_PATH${DATASET_NAME}2K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_partitioner input.mp4

echo "----------------"
echo "4K Input"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_partitioner input.mp4

rm input.mp4
rm tile*.mp4