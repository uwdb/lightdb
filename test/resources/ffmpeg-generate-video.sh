# $1 - color
# $2 - duration
# $3 - frames per second


ffmpeg -f lavfi -i color=c=$1@0.2:duration=$2:s=qcif:r=$3 $1.mp4
ffmpeg -i $1.mp4 -c copy $1.h264
