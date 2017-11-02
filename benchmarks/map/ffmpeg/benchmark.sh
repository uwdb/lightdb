#!/usr/bin/env bash

echo "----------------"
echo "1K Input"

echo "20 seconds"
time ./greyscale.sh ../../../test/resources/test-1K-20s.h264 out.hevc

echo "40 seconds"
time ./greyscale.sh ../../../test/resources/test-1K-40s.h264 out.hevc

echo "60 seconds"
time ./greyscale.sh ../../../test/resources/test-1K-60s.h264 out.hevc

echo "----------------"
echo "2K Input"

echo "20 seconds"
time ./greyscale.sh ../../../test/resources/test-2K-20s.h264 out.hevc

echo "40 seconds"
time ./greyscale.sh ../../../test/resources/test-2K-40s.h264 out.hevc

echo "60 seconds"
time ./greyscale.sh ../../../test/resources/test-2K-60s.h264 out.hevc

echo "----------------"
echo "4K Input"

echo "20 seconds"
time ./greyscale.sh ../../../test/resources/test-4K-20s.h264 out.hevc

echo "40 seconds"
time ./greyscale.sh ../../../test/resources/test-4K-40s.h264 out.hevc

echo "60 seconds"
time ./greyscale.sh ../../../test/resources/test-4K-60s.h264 out.hevc

rm out.hevc