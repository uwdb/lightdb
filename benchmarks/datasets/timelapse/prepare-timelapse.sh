# Codecs

ffmpeg -y -i ../videos/CIw8R8thnm8.mkv -c copy -an -t 00:01:30 timelapse4K.h264

# Resolutions varying on 25 second video

ffmpeg -y -i timelapse4K.h264 -vf "scale=iw/2:ih/2" timelapse2K.h264

ffmpeg -y -i timelapse2K.h264 -vf "scale=iw/2:ih/2" timelapse1K.h264

#Times

ffmpeg -y -i timelapse4K.h264 -t 20 -c copy timelapse20seconds.h264

ffmpeg -y -i timelapse4K.h264 -t 15 -c copy timelapse15seconds.h264

ffmpeg -y -i timelapse4K.h264 -t 10 -c copy timelapse10seconds.h264

ffmpeg -y -i timelapse4K.h264 -t 5 -c copy timelapse5seconds.h264

# Codecs

ffmpeg -y -i timelapse4K.h264 -c:v libx265 -preset ultrafast -x265-params lossless=1 timelapse4Khevc.h265

#QP

ffmpeg -y -i timelapse4K.h264 -c:v libx264 -qp 0 timelapse0qp.h264

ffmpeg -y -i timelapse4K.h264 -c:v libx264 -qp 18 timelapse18qp.h264

ffmpeg -y -i timelapse4K.h264 -c:v libx264 -qp 23 timelapse23qp.h264

ffmpeg -y -i timelapse4K.h264 -c:v libx264 -qp 28 timelapse28qp.h264

ffmpeg -y -i timelapse4K.h264 -c:v libx264 -qp 41 timelapse41qp.h264

ffmpeg -y -i timelapse4K.h264 -c:v libx264 -qp 51 timelapse51qp.h264

# GOP
ffmpeg -y -i timelapse4K.h264 -g 15 timelapse15g.h264

ffmpeg -y -i timelapse4K.h264 -g 30 timelapse30g.h264

ffmpeg -y -i timelapse4K.h264 -g 45 timelapse45g.h264

ffmpeg -y -i timelapse4K.h264 -g 60 timelapse60g.h264


# B frames
ffmpeg -y -i timelapse4K.h264 -bf 0 timelapse0bf.h264

ffmpeg -y -i timelapse4K.h264 -bf 2 timelapse2bf.h264

ffmpeg -y -i timelapse4K.h264 -bf 4 timelapse4bf.h264

ffmpeg -y -i timelapse4K.h264 -bf 8 timelapse8bf.h264
