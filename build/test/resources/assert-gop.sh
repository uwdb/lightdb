#!/bin/sh

input=$1
expected=$2
GOP=0;

ffprobe -hide_banner -loglevel warning -show_frames $input |
while read -r p; do
  if [ "$p" = "key_frame=0" ]
  then
    GOP=$((GOP+1))
  fi

  if [ "$p" = "key_frame=1" ]
  then
    if [ $((GOP+1)) != $expected ] && [ $GOP != 0 ]
    then
        echo "ERROR: found GOP $((GOP+1)), expected $expected"
        exit ${GOP}
    fi

    GOP=0;
  fi
done