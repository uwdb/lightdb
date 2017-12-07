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

time ./crop.sh $file $width $height $left $top out.h264

echo "----------------"
echo "2K Input"

width=768
height=768
left=256
top=256
file=$DATASET_PATH${DATASET_NAME}2K.$DATASET_EXTENSION

time ./crop.sh $file $width $height $left $top out.h264

echo "----------------"
echo "4K Input"

width=1440
height=1440
left=480
top=480
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./crop.sh $file $width $height $left $top out.h264

rm out.h264
