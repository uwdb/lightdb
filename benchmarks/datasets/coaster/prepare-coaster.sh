set -e

wget http://dash.ipv6.enstb.fr/headMovements/archives/videos.tar
tar xf videos.tar
ffmpeg -y -i videos/8lsB-P8nGSM.mkv -c copy -an -s 00:01:05 -t 00:01:30 coaster4K.h264
ffmpeg -y -i coaster4K.h264 -vf "scale=iw/2:ih/2" coaster2K.h264
ffmpeg -y -i coaster2K.h264 -vf "scale=iw/2:ih/2" coaster1K.h264
rm -rf videos
rm videos.tar
