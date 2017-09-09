#!/bin/bash

ffmpeg -i $1 -i $2 -filter_complex psnr -f null -
