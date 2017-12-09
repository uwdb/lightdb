#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "1K Input"

file=$DATASET_PATH${DATASET_NAME}1K.$DATASET_EXTENSION

#time ./greyscale.sh $file out.h264

echo "----------------"
echo "2K Input"

file=$DATASET_PATH${DATASET_NAME}2K.$DATASET_EXTENSION

time ./greyscale.sh $file out.h264

echo "----------------"
echo "4K Input"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./greyscale.sh $file out.h264

rm out.h264
