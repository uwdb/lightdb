#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "----------------"
echo "4K Input, overlay Union(L, L)"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -filter:v "crop=in_w:in_h:0:0" input.mp4
time ./opencv_union overlay input.mp4 input.mp4

echo "----------------"
echo "4K Input, stack (TileUnion)"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -loglevel error -y -i $file -filter:v "crop=in_w/2:in_h:0:0" input.mp4
time ./opencv_union stack input.mp4

echo "----------------"
echo "4K Input, concat (GopUnion)"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION
segment_time=1
segment_count=90
ffmpeg -hide_banner -y -loglevel error -vcodec h264_cuvid -i $file \
  -vcodec h264_nvenc -r 30 -g 30 \
  -f segment -segment_time $segment_time -segment_time_delta 0.05 -segment_format mp4 segment%d.mp4

time ./opencv_union gop $segment_count

rm segment*.mp4
rm input.mp4
