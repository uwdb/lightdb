set -e

wget http://dash.ipv6.enstb.fr/headMovements/archives/videos.tar
tar xf videos.tar
ffmpeg -y -i videos/s-AJRFQuAtE.mkv -an -vf "scale=3840:2048" -t 00:01:30 venice4K.h264
ffmpeg -y -i venice4K.h264 -vf "scale=iw/2:ih/2" venice2K.h264
ffmpeg -y -i venice2K.h264 -vf "scale=iw/2:ih/2" venice1K.h264
rm -rf videos
rm videos.tar
