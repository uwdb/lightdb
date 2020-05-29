#!/bin/bash

WIDTH=$(ffprobe -v error -of flat -show_frames $1 | grep width | cut -d = -f 2 | head -n 1)
HEIGHT=$(ffprobe -v error -of flat -show_frames $1 | grep height | cut -d = -f 2 | head -n 1)

if [ $? -ne 0 ];
then
    echo "ERROR: unable to parse video $1"
    exit -1
elif [[ $HEIGHT -ne $2 || $WIDTH -ne $3 ]];
then
    echo "ERROR: found resolution ($WIDTH, $HEIGHT), expected ($3, $2)"
    exit -1
fi
