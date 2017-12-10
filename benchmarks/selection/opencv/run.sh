#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input, t=[2.5, 4.5]"

width=3840
height=2048
start=2.5
end=4.5
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
time ./opencv_selector time input.mp4 $width $height $start $end

echo "----------------"
echo "4K Input, theta=[pi/2]"

width=1920
height=2048
left=1920
top=0
file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
#time ./opencv_selector crop input.mp4 $width $height $left $top

echo "----------------"
echo "4K Input, theta=[pi/2, pi], phi=[pi/4, pi/2]"

width=960
height=512
left=960
top=512
ffmpeg -hide_banner -loglevel error -y -i $file -c copy input.mp4
#time ./opencv_selector crop input.mp4 $width $height $left $top

rm input.mp4
