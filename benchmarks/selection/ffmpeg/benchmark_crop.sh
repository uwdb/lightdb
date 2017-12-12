#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input, theta=[pi/2, pi]"

width=1920
height=2048
left=1920
top=0
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./crop.sh $file $width $height $left $top out.h264

echo "----------------"
echo "4K Input, theta=[pi/2, pi], phi=[pi/4, pi/2]"

width=960
height=512
left=960
top=512
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

time ./crop.sh $file $width $height $left $top out.h264

echo "----------------"
echo "4K 4x4 tiled Input, tile 0,0"

rows=4
cols=4
row=0
col=0
file=tiled.hevc

time ./select_tile.sh $file $col $row $rows $cols out.hevc

rm out.h264
