#!/usr/bin/env bash

echo "----------------"
echo "1K Input"

width=384
height=384
left=128
top=128

echo "20 seconds"
time ./crop.sh ../../../test/resources/test-1K-20s.h264 $width $height $left $top out.h264

echo "40 seconds"
time ./crop.sh ../../../test/resources/test-1K-40s.h264 $width $height $left $top out.h264

echo "60 seconds"
time ./crop.sh ../../../test/resources/test-1K-60s.h264 $width $height $left $top out.h264

echo "----------------"
echo "2K Input"

width=768
height=768
left=256
top=256

echo "20 seconds"
time ./crop.sh ../../../test/resources/test-2K-20s.h264 $width $height $left $top out.h264

echo "40 seconds"
time ./crop.sh ../../../test/resources/test-2K-40s.h264 $width $height $left $top out.h264

echo "60 seconds"
time ./crop.sh ../../../test/resources/test-2K-60s.h264 $width $height $left $top out.h264

echo "----------------"
echo "4K Input"

width=1440
height=1440
left=480
top=480

echo "20 seconds"
time ./crop.sh ../../../test/resources/test-4K-20s.h264 $width $height $left $top out.h264

echo "40 seconds"
time ./crop.sh ../../../test/resources/test-4K-40s.h264 $width $height $left $top out.h264

echo "60 seconds"
time ./crop.sh ../../../test/resources/test-4K-60s.h264 $width $height $left $top out.h264

rm out.h264