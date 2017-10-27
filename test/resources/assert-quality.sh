#!/bin/bash

PSNR_LIMIT=$3
MEAN_PSNR="$(ffmpeg -i $1 -i $2 -filter_complex psnr -f null - 2>&1 | \
  grep average |                                                      \
  tail -n 1 |                                                         \
  sed -rn 's/.*average:(([[:digit:].]+)|inf).*/\1/p')"

if [ $? -ne 0 ];
then
    echo "ERROR: unable to parse video $1"
    exit -1
fi

if [ "$MEAN_PSNR" = "inf" ];
then
    IS_VALID=1
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