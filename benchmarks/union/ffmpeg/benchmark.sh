#!/usr/bin/env bash

DATASET_NAME=timelapse
DATASET_PATH=../../datasets/$DATASET_NAME/
DATASET_EXTENSION=h264
echo Dataset: $DATASET_NAME

echo "================"
echo "SIDE BY SIDE HEVC->HEVC"

echo "----------------"
echo "1K Input"

file=$DATASET_PATH${DATASET_NAME}1K.$DATASET_EXTENSION

ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-1K-60s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "----------------"
echo "2K Input"

file=$DATASET_PATH${DATASET_NAME}2K.$DATASET_EXTENSION

ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-2K-60s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "----------------"
echo "4K Input"

file=$DATASET_PATH${DATASET_NAME}4K.$DATASET_EXTENSION

ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-4K-60s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

rm input.hevc
rm out.hevc
