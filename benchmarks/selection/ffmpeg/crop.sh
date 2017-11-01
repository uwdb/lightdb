#!/usr/bin/env bash

ffmpeg -hide_banner -y -loglevel error  -i $1 -filter:v "crop=$2:$3:$4:$5" $6
