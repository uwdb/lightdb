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

time ./crop.sh $file $width $height $left $top out.h264

echo "----------------"
echo "2K Input"

width=1920
height=1024
left=0
top=0
file=$DATASET_PATH${DATASET_NAME}2K.$DATASET_EXTENSION

time ./crop.sh $file $width $height $left $top out.h264

echo "----------------"
echo "4K Input"

width=3840
height=2048
left=0
top=0
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./crop.sh $file $width $height $left $top out.h264

rm out.h264
