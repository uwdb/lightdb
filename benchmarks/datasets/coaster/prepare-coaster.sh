set -e 

# Codecs

ffmpeg -y -i ../videos/8lsB-P8nGSM.mkv -c copy -an -s 00:01:05 -t 00:01:30 coaster4K.h264

# Resolutions varying on 25 second video

ffmpeg -y -i coaster4K.h264 -vf "scale=iw/2:ih/2" coaster2K.h264

ffmpeg -y -i coaster2K.h264 -vf "scale=iw/2:ih/2" coaster1K.h264

#Times

ffmpeg -y -i coaster4K.h264 -t 20 -c copy coaster20seconds.h264

ffmpeg -y -i coaster4K.h264 -t 15 -c copy coaster15seconds.h264

ffmpeg -y -i coaster4K.h264 -t 10 -c copy coaster10seconds.h264

ffmpeg -y -i coaster4K.h264 -t 5 -c copy coaster5seconds.h264

# Codecs

ffmpeg -y -i coaster4K.h264 -c:v libx265 -preset ultrafast -x265-params lossless=1 coaster4Khevc.h265

#QP

ffmpeg -y -i coaster4K.h264 -c:v libx264 -qp 0 coaster0qp.h264

ffmpeg -y -i coaster4K.h264 -c:v libx264 -qp 18 coaster18qp.h264

ffmpeg -y -i coaster4K.h264 -c:v libx264 -qp 23 coaster23qp.h264

ffmpeg -y -i coaster4K.h264 -c:v libx264 -qp 28 coaster28qp.h264

ffmpeg -y -i coaster4K.h264 -c:v libx264 -qp 41 coaster41qp.h264

ffmpeg -y -i coaster4K.h264 -c:v libx264 -qp 51 coaster51qp.h264

# GOP
ffmpeg -y -i coaster4K.h264 -g 15 coaster15g.h264

ffmpeg -y -i coaster4K.h264 -g 30 coaster30g.h264

ffmpeg -y -i coaster4K.h264 -g 45 coaster45g.h264

ffmpeg -y -i coaster4K.h264 -g 60 coaster60g.h264


# B frames
ffmpeg -y -i coaster4K.h264 -bf 0 coaster0bf.h264

ffmpeg -y -i coaster4K.h264 -bf 2 coaster2bf.h264

ffmpeg -y -i coaster4K.h264 -bf 4 coaster4bf.h264

ffmpeg -y -i coaster4K.h264 -bf 8 coaster8bf.h264
