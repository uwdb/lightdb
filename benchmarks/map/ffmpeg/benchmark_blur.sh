#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./blur.sh $file out.h264

rm out.h264
