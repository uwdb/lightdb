#!/bin/bash

INPUT=$1
REFERENCE=$2
PSNR_LIMIT=$3
REFERENCE_LEFT=$4
REFERENCE_TOP=$5
REFERENCE_RIGHT=$6
REFERENCE_BOTTOM=$7
SOURCE_LEFT=$8
SOURCE_TOP=$9
SOURCE_RIGHT=${10}
SOURCE_BOTTOM=${11}

REFERENCE_INTERMEDIATE=$(mktemp --suffix .h264)
SOURCE_INTERMEDIATE=$(mktemp --suffix .h264)

if [ $REFERENCE_LEFT -ne -1 ];
then
  CROP="$(ffmpeg -y -v error -i $REFERENCE -crf 0 -filter:v \
          crop=$REFERENCE_RIGHT-$REFERENCE_LEFT:$REFERENCE_BOTTOM-$REFERENCE_TOP:$REFERENCE_LEFT:$REFERENCE_TOP \
          $REFERENCE_INTERMEDIATE)"
else
  CROP="$(cp $REFERENCE $REFERENCE_INTERMEDIATE)"
fi

if [ $? -ne 0 ];
then
    echo "ERROR: unable to parse reference video $REFERENCE"
    exit -1
fi

if [ $SOURCE_LEFT -ne -1 ];
then
  CROP="$(ffmpeg -y -v error -i $INPUT -crf 0 -filter:v \
          crop=$SOURCE_RIGHT-$SOURCE_LEFT:$SOURCE_BOTTOM-$SOURCE_TOP:$SOURCE_LEFT:$SOURCE_TOP \
          $SOURCE_INTERMEDIATE)"
else
  CROP="$(ffmpeg -y -v error -i $INPUT -crf 0 $SOURCE_INTERMEDIATE)"
fi

if [ $? -ne 0 ];
then
    echo "ERROR: unable to parse reference video $INPUT"
    exit -1
fi

MEAN_PSNR="$(ffmpeg -i $SOURCE_INTERMEDIATE -i $REFERENCE_INTERMEDIATE \
                    -lavfi '[0:v]format=gbrp14,setpts=N[out0];[1:v]format=gbrp14,setpts=N[out1];[out0][out1]psnr' \
                    -f null - 2>&1 | \
  grep average |                                                      \
  tail -n 1 |                                                         \
  sed -rn 's/.*average:(([[:digit:].]+)|inf).*/\1/p')"

if [ $? -ne 0 ];
then
    echo "ERROR: error generating PSNR for intermediate videos"
    exit -1
fi

rm $SOURCE_INTERMEDIATE
rm $REFERENCE_INTERMEDIATE

IS_VALID=0
if [ "$MEAN_PSNR" = "inf" ];
then
    IS_VALID=1
elif [ "$MEAN_PSNR" = "" ];
then
    IS_VALID=0
    MEAN_PSNR="[NaN]"
else
    IS_VALID=$(bc <<< "$MEAN_PSNR >= $PSNR_LIMIT")
fi

if [ $? -ne 0 ];
then
    echo "ERROR: unable to compare PSNRs"
    exit -1
elif [ $IS_VALID -eq 0 ];
then
    echo "ERROR: calculated PSNR $MEAN_PSNR below limit of $PSNR_LIMIT"
    exit -1
fi