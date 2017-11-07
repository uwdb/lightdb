#!/usr/bin/env bash

echo "================"
echo "SIDE BY SIDE HEVC->HEVC"

echo "----------------"
echo "1K Input"

echo "20 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-1K-20s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "40 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-1K-40s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "60 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-1K-60s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "----------------"
echo "2K Input"

echo "20 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-2K-20s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "40 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-2K-40s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "60 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-2K-60s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "----------------"
echo "4K Input"

echo "20 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-4K-20s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "40 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-4K-40s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

echo "60 seconds"
ffmpeg -hide_banner -y -loglevel error -i ../../../test/resources/test-4K-60s.h264 -vcodec hevc_nvenc input.hevc
/usr/bin/time --format "%E" ./side-by-side.sh input.hevc out.hevc

rm input.hevc
rm out.hevc