#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input, theta partition"

width=3840
height=2048
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./partition_theta.sh $file $width $height out.h264

echo "----------------"
echo "4K Input, phi partition"

width=3840
height=2048
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./partition_phi.sh $file $width $height out.h264

rm *out.h264
