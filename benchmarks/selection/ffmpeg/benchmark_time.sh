#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input"

start=2
end=4
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./time.sh $file $start $end out.h264

rm out.h264
