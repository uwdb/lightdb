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

time ./crop.sh $file $width $height $left $top out.h264

echo "----------------"
echo "4K Input, redundant select"

width=1920
height=1024
left=1920
top=1024
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

#time ./redundant_crop.sh $file $width $height $left $top out.h264

rm out.h264
