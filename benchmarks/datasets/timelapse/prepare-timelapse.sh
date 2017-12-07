set -e

wget http://dash.ipv6.enstb.fr/headMovements/archives/videos.tar
tar xf videos.tar
ffmpeg -y -i videos/CIw8R8thnm8.mkv -c copy -an -t 00:01:30 timelapse4K.h264
ffmpeg -y -i timelapse4K.mp4 -vf "scale=iw/2:ih/2" timelapse2K.h264
ffmpeg -y -i timelapse2K.mp4 -vf "scale=iw/2:ih/2" timelapse1K.h264
rm -rf videos
rm videos.tar
