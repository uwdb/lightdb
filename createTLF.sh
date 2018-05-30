FILEPATH=$1
TLFNAME=$2

mkdir ~/lightdb/build/test/resources/$TLFNAME
cp $FILEPATH ~/lightdb/build/test/resources/$TLFNAME/stream0.h264
ffmpeg -i ~/lightdb/build/test/resources/$TLFNAME/stream0.h264 -c copy ~/lightdb/build/test/resources/$TLFNAME/metadata.mp4
