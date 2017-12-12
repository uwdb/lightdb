#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input, [0, 6]"

start=0
end=6
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./time.sh $file $start $end out.h264

echo "----------------"
echo "4K Input, [2.5, 4.5]"

start=2.5
end=4.5
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./time.sh $file $start $end out.h264

rm out.h264
