#!/bin/bash

FRAMES="$(ffprobe -v error -count_frames -select_streams v:0 \
        -show_entries stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 \
        $1)"

if [ $? -ne 0 ];
then
    echo "ERROR: unable to parse video $1"
    exit -1
elif [ "$FRAMES" = "N/A" ];
then
    echo "ERROR: no frames found in $1"
    exit -1
elif [ $FRAMES -ne "$2" ];
then
    echo "ERROR: found $FRAMES frames, expected $2"
    exit $FRAMES
fi