#!/usr/bin/env bash

echo "----------------"
echo "1K Input"

width=384
height=384
left=128
top=128

echo "20 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-1K-20s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "40 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-1K-40s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "60 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-1K-60s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "----------------"
echo "2K Input"

width=768
height=768
left=256
top=256

echo "20 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-2K-20s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "40 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-2K-40s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "60 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-2K-60s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "----------------"
echo "4K Input"

width=1440
height=1440
left=480
top=480

echo "20 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-4K-20s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "40 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-4K-40s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top

echo "60 seconds"
ffmpeg -hide_banner -loglevel error -y -i ../../../test/resources/test-4K-60s.h264 -c copy input.mp4
time ./opencv_selector input.mp4 $width $height $left $top



