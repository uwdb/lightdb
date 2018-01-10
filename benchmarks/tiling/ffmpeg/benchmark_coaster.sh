# height: 2048, width: 3840
# rows: 4, columns: 4
#tile width: 960, tile height: 512
ffmpeg \
  -i coaster.h264 \
  -vcodec h264_cuvid \
  -filter_complex "
    [0:v]crop=960:512:0:0[tile0];
    [0:v]crop=960:512:960:0[tile1];
    [0:v]crop=960:512:1920:0[tile2];
    [0:v]crop=960:512:2880:0[tile3];
    [0:v]crop=960:512:0:512[tile4];
    [0:v]crop=960:512:960:512[tile5];
    [0:v]crop=960:512:1920:512[tile6];
    [0:v]crop=960:512:2880:512[tile7];
    [0:v]crop=960:512:0:1024[tile8];
    [0:v]crop=960:512:960:1024[tile9];
    [0:v]crop=960:512:1920:1024[tile10];
    [0:v]crop=960:512:2880:1024[tile11];
    [0:v]crop=960:512:0:1536[tile12];
    [0:v]crop=960:512:960:1536[tile13];
    [0:v]crop=960:512:1920:1536[tile14];
    [0:v]crop=960:512:2880:1536[tile15]" \
  -map "[tile0]" \
    -vcodec h264_nvenc \
    -b:v 1000k \
    -minrate 1000k \
    -maxrate 1000k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-0-%d.mp4 \
  -map "[tile1]" \
    -vcodec h264_nvenc \
    -b:v 5000k \
    -minrate 5000k \
    -maxrate 5000k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-1-%d.mp4 \
  -map "[tile2]" \
    -vcodec h264_nvenc \
    -b:v 1000k \
    -minrate 1000k \
    -maxrate 1000k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-2-%d.mp4 \
  -map "[tile3]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-3-%d.mp4 \
  -map "[tile4]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-4-%d.mp4 \
  -map "[tile5]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-5-%d.mp4 \
  -map "[tile6]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-6-%d.mp4 \
  -map "[tile7]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-7-%d.mp4 \
  -map "[tile8]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-8-%d.mp4 \
  -map "[tile9]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-9-%d.mp4 \
  -map "[tile10]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-10-%d.mp4 \
  -map "[tile11]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-11-%d.mp4 \
  -map "[tile12]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-12-%d.mp4 \
  -map "[tile13]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-13-%d.mp4 \
  -map "[tile14]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-14-%d.mp4 \
  -map "[tile15]" \
    -vcodec h264_nvenc \
    -b:v 50k \
    -minrate 50k \
    -maxrate 50k \
    -r 30 -g 30 \
    -f segment \
    -segment_format mp4 \
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \
    -segment_time 1 \
    -segment_time_delta 0.05 \
    pipe-15-%d.mp4 \

  

ffmpeg \
  -y \
  -r 15 \
\
 -i pipe-0-0.mp4 \
\
 -i pipe-0-1.mp4 \
\
 -i pipe-0-2.mp4 \
\
 -i pipe-0-3.mp4 \
\
 -i pipe-0-4.mp4 \
\
 -i pipe-0-5.mp4 \
\
 -i pipe-0-6.mp4 \
\
 -i pipe-0-7.mp4 \
\
 -i pipe-0-8.mp4 \
\
 -i pipe-0-9.mp4 \
\
 -i pipe-0-10.mp4 \
\
 -i pipe-0-11.mp4 \
\
 -i pipe-0-12.mp4 \
\
 -i pipe-0-13.mp4 \
\
 -i pipe-0-14.mp4 \
\
 -i pipe-0-15.mp4 \
\
 -i pipe-0-16.mp4 \
\
 -i pipe-0-17.mp4 \
\
 -i pipe-0-18.mp4 \
\
 -i pipe-0-19.mp4 \
\
 -i pipe-0-20.mp4 \
\
 -i pipe-0-21.mp4 \
\
 -i pipe-0-22.mp4 \
\
 -i pipe-0-23.mp4 \
\
 -i pipe-0-24.mp4 \
\
 -i pipe-0-25.mp4 \
\
 -i pipe-0-26.mp4 \
\
 -i pipe-0-27.mp4 \
\
 -i pipe-0-28.mp4 \
\
 -i pipe-0-29.mp4 \
\
 -i pipe-0-30.mp4 \
\
 -i pipe-0-31.mp4 \
\
 -i pipe-0-32.mp4 \
\
 -i pipe-0-33.mp4 \
\
 -i pipe-0-34.mp4 \
\
 -i pipe-0-35.mp4 \
\
 -i pipe-0-36.mp4 \
\
 -i pipe-0-37.mp4 \
\
 -i pipe-0-38.mp4 \
\
 -i pipe-0-39.mp4 \
\
 -i pipe-1-0.mp4 \
\
 -i pipe-1-1.mp4 \
\
 -i pipe-1-2.mp4 \
\
 -i pipe-1-3.mp4 \
\
 -i pipe-1-4.mp4 \
\
 -i pipe-1-5.mp4 \
\
 -i pipe-1-6.mp4 \
\
 -i pipe-1-7.mp4 \
\
 -i pipe-1-8.mp4 \
\
 -i pipe-1-9.mp4 \
\
 -i pipe-1-10.mp4 \
\
 -i pipe-1-11.mp4 \
\
 -i pipe-1-12.mp4 \
\
 -i pipe-1-13.mp4 \
\
 -i pipe-1-14.mp4 \
\
 -i pipe-1-15.mp4 \
\
 -i pipe-1-16.mp4 \
\
 -i pipe-1-17.mp4 \
\
 -i pipe-1-18.mp4 \
\
 -i pipe-1-19.mp4 \
\
 -i pipe-1-20.mp4 \
\
 -i pipe-1-21.mp4 \
\
 -i pipe-1-22.mp4 \
\
 -i pipe-1-23.mp4 \
\
 -i pipe-1-24.mp4 \
\
 -i pipe-1-25.mp4 \
\
 -i pipe-1-26.mp4 \
\
 -i pipe-1-27.mp4 \
\
 -i pipe-1-28.mp4 \
\
 -i pipe-1-29.mp4 \
\
 -i pipe-1-30.mp4 \
\
 -i pipe-1-31.mp4 \
\
 -i pipe-1-32.mp4 \
\
 -i pipe-1-33.mp4 \
\
 -i pipe-1-34.mp4 \
\
 -i pipe-1-35.mp4 \
\
 -i pipe-1-36.mp4 \
\
 -i pipe-1-37.mp4 \
\
 -i pipe-1-38.mp4 \
\
 -i pipe-1-39.mp4 \
\
 -i pipe-2-0.mp4 \
\
 -i pipe-2-1.mp4 \
\
 -i pipe-2-2.mp4 \
\
 -i pipe-2-3.mp4 \
\
 -i pipe-2-4.mp4 \
\
 -i pipe-2-5.mp4 \
\
 -i pipe-2-6.mp4 \
\
 -i pipe-2-7.mp4 \
\
 -i pipe-2-8.mp4 \
\
 -i pipe-2-9.mp4 \
\
 -i pipe-2-10.mp4 \
\
 -i pipe-2-11.mp4 \
\
 -i pipe-2-12.mp4 \
\
 -i pipe-2-13.mp4 \
\
 -i pipe-2-14.mp4 \
\
 -i pipe-2-15.mp4 \
\
 -i pipe-2-16.mp4 \
\
 -i pipe-2-17.mp4 \
\
 -i pipe-2-18.mp4 \
\
 -i pipe-2-19.mp4 \
\
 -i pipe-2-20.mp4 \
\
 -i pipe-2-21.mp4 \
\
 -i pipe-2-22.mp4 \
\
 -i pipe-2-23.mp4 \
\
 -i pipe-2-24.mp4 \
\
 -i pipe-2-25.mp4 \
\
 -i pipe-2-26.mp4 \
\
 -i pipe-2-27.mp4 \
\
 -i pipe-2-28.mp4 \
\
 -i pipe-2-29.mp4 \
\
 -i pipe-2-30.mp4 \
\
 -i pipe-2-31.mp4 \
\
 -i pipe-2-32.mp4 \
\
 -i pipe-2-33.mp4 \
\
 -i pipe-2-34.mp4 \
\
 -i pipe-2-35.mp4 \
\
 -i pipe-2-36.mp4 \
\
 -i pipe-2-37.mp4 \
\
 -i pipe-2-38.mp4 \
\
 -i pipe-2-39.mp4 \
\
 -i pipe-3-0.mp4 \
\
 -i pipe-3-1.mp4 \
\
 -i pipe-3-2.mp4 \
\
 -i pipe-3-3.mp4 \
\
 -i pipe-3-4.mp4 \
\
 -i pipe-3-5.mp4 \
\
 -i pipe-3-6.mp4 \
\
 -i pipe-3-7.mp4 \
\
 -i pipe-3-8.mp4 \
\
 -i pipe-3-9.mp4 \
\
 -i pipe-3-10.mp4 \
\
 -i pipe-3-11.mp4 \
\
 -i pipe-3-12.mp4 \
\
 -i pipe-3-13.mp4 \
\
 -i pipe-3-14.mp4 \
\
 -i pipe-3-15.mp4 \
\
 -i pipe-3-16.mp4 \
\
 -i pipe-3-17.mp4 \
\
 -i pipe-3-18.mp4 \
\
 -i pipe-3-19.mp4 \
\
 -i pipe-3-20.mp4 \
\
 -i pipe-3-21.mp4 \
\
 -i pipe-3-22.mp4 \
\
 -i pipe-3-23.mp4 \
\
 -i pipe-3-24.mp4 \
\
 -i pipe-3-25.mp4 \
\
 -i pipe-3-26.mp4 \
\
 -i pipe-3-27.mp4 \
\
 -i pipe-3-28.mp4 \
\
 -i pipe-3-29.mp4 \
\
 -i pipe-3-30.mp4 \
\
 -i pipe-3-31.mp4 \
\
 -i pipe-3-32.mp4 \
\
 -i pipe-3-33.mp4 \
\
 -i pipe-3-34.mp4 \
\
 -i pipe-3-35.mp4 \
\
 -i pipe-3-36.mp4 \
\
 -i pipe-3-37.mp4 \
\
 -i pipe-3-38.mp4 \
\
 -i pipe-3-39.mp4 \
\
 -i pipe-4-0.mp4 \
\
 -i pipe-4-1.mp4 \
\
 -i pipe-4-2.mp4 \
\
 -i pipe-4-3.mp4 \
\
 -i pipe-4-4.mp4 \
\
 -i pipe-4-5.mp4 \
\
 -i pipe-4-6.mp4 \
\
 -i pipe-4-7.mp4 \
\
 -i pipe-4-8.mp4 \
\
 -i pipe-4-9.mp4 \
\
 -i pipe-4-10.mp4 \
\
 -i pipe-4-11.mp4 \
\
 -i pipe-4-12.mp4 \
\
 -i pipe-4-13.mp4 \
\
 -i pipe-4-14.mp4 \
\
 -i pipe-4-15.mp4 \
\
 -i pipe-4-16.mp4 \
\
 -i pipe-4-17.mp4 \
\
 -i pipe-4-18.mp4 \
\
 -i pipe-4-19.mp4 \
\
 -i pipe-4-20.mp4 \
\
 -i pipe-4-21.mp4 \
\
 -i pipe-4-22.mp4 \
\
 -i pipe-4-23.mp4 \
\
 -i pipe-4-24.mp4 \
\
 -i pipe-4-25.mp4 \
\
 -i pipe-4-26.mp4 \
\
 -i pipe-4-27.mp4 \
\
 -i pipe-4-28.mp4 \
\
 -i pipe-4-29.mp4 \
\
 -i pipe-4-30.mp4 \
\
 -i pipe-4-31.mp4 \
\
 -i pipe-4-32.mp4 \
\
 -i pipe-4-33.mp4 \
\
 -i pipe-4-34.mp4 \
\
 -i pipe-4-35.mp4 \
\
 -i pipe-4-36.mp4 \
\
 -i pipe-4-37.mp4 \
\
 -i pipe-4-38.mp4 \
\
 -i pipe-4-39.mp4 \
\
 -i pipe-5-0.mp4 \
\
 -i pipe-5-1.mp4 \
\
 -i pipe-5-2.mp4 \
\
 -i pipe-5-3.mp4 \
\
 -i pipe-5-4.mp4 \
\
 -i pipe-5-5.mp4 \
\
 -i pipe-5-6.mp4 \
\
 -i pipe-5-7.mp4 \
\
 -i pipe-5-8.mp4 \
\
 -i pipe-5-9.mp4 \
\
 -i pipe-5-10.mp4 \
\
 -i pipe-5-11.mp4 \
\
 -i pipe-5-12.mp4 \
\
 -i pipe-5-13.mp4 \
\
 -i pipe-5-14.mp4 \
\
 -i pipe-5-15.mp4 \
\
 -i pipe-5-16.mp4 \
\
 -i pipe-5-17.mp4 \
\
 -i pipe-5-18.mp4 \
\
 -i pipe-5-19.mp4 \
\
 -i pipe-5-20.mp4 \
\
 -i pipe-5-21.mp4 \
\
 -i pipe-5-22.mp4 \
\
 -i pipe-5-23.mp4 \
\
 -i pipe-5-24.mp4 \
\
 -i pipe-5-25.mp4 \
\
 -i pipe-5-26.mp4 \
\
 -i pipe-5-27.mp4 \
\
 -i pipe-5-28.mp4 \
\
 -i pipe-5-29.mp4 \
\
 -i pipe-5-30.mp4 \
\
 -i pipe-5-31.mp4 \
\
 -i pipe-5-32.mp4 \
\
 -i pipe-5-33.mp4 \
\
 -i pipe-5-34.mp4 \
\
 -i pipe-5-35.mp4 \
\
 -i pipe-5-36.mp4 \
\
 -i pipe-5-37.mp4 \
\
 -i pipe-5-38.mp4 \
\
 -i pipe-5-39.mp4 \
\
 -i pipe-6-0.mp4 \
\
 -i pipe-6-1.mp4 \
\
 -i pipe-6-2.mp4 \
\
 -i pipe-6-3.mp4 \
\
 -i pipe-6-4.mp4 \
\
 -i pipe-6-5.mp4 \
\
 -i pipe-6-6.mp4 \
\
 -i pipe-6-7.mp4 \
\
 -i pipe-6-8.mp4 \
\
 -i pipe-6-9.mp4 \
\
 -i pipe-6-10.mp4 \
\
 -i pipe-6-11.mp4 \
\
 -i pipe-6-12.mp4 \
\
 -i pipe-6-13.mp4 \
\
 -i pipe-6-14.mp4 \
\
 -i pipe-6-15.mp4 \
\
 -i pipe-6-16.mp4 \
\
 -i pipe-6-17.mp4 \
\
 -i pipe-6-18.mp4 \
\
 -i pipe-6-19.mp4 \
\
 -i pipe-6-20.mp4 \
\
 -i pipe-6-21.mp4 \
\
 -i pipe-6-22.mp4 \
\
 -i pipe-6-23.mp4 \
\
 -i pipe-6-24.mp4 \
\
 -i pipe-6-25.mp4 \
\
 -i pipe-6-26.mp4 \
\
 -i pipe-6-27.mp4 \
\
 -i pipe-6-28.mp4 \
\
 -i pipe-6-29.mp4 \
\
 -i pipe-6-30.mp4 \
\
 -i pipe-6-31.mp4 \
\
 -i pipe-6-32.mp4 \
\
 -i pipe-6-33.mp4 \
\
 -i pipe-6-34.mp4 \
\
 -i pipe-6-35.mp4 \
\
 -i pipe-6-36.mp4 \
\
 -i pipe-6-37.mp4 \
\
 -i pipe-6-38.mp4 \
\
 -i pipe-6-39.mp4 \
\
 -i pipe-7-0.mp4 \
\
 -i pipe-7-1.mp4 \
\
 -i pipe-7-2.mp4 \
\
 -i pipe-7-3.mp4 \
\
 -i pipe-7-4.mp4 \
\
 -i pipe-7-5.mp4 \
\
 -i pipe-7-6.mp4 \
\
 -i pipe-7-7.mp4 \
\
 -i pipe-7-8.mp4 \
\
 -i pipe-7-9.mp4 \
\
 -i pipe-7-10.mp4 \
\
 -i pipe-7-11.mp4 \
\
 -i pipe-7-12.mp4 \
\
 -i pipe-7-13.mp4 \
\
 -i pipe-7-14.mp4 \
\
 -i pipe-7-15.mp4 \
\
 -i pipe-7-16.mp4 \
\
 -i pipe-7-17.mp4 \
\
 -i pipe-7-18.mp4 \
\
 -i pipe-7-19.mp4 \
\
 -i pipe-7-20.mp4 \
\
 -i pipe-7-21.mp4 \
\
 -i pipe-7-22.mp4 \
\
 -i pipe-7-23.mp4 \
\
 -i pipe-7-24.mp4 \
\
 -i pipe-7-25.mp4 \
\
 -i pipe-7-26.mp4 \
\
 -i pipe-7-27.mp4 \
\
 -i pipe-7-28.mp4 \
\
 -i pipe-7-29.mp4 \
\
 -i pipe-7-30.mp4 \
\
 -i pipe-7-31.mp4 \
\
 -i pipe-7-32.mp4 \
\
 -i pipe-7-33.mp4 \
\
 -i pipe-7-34.mp4 \
\
 -i pipe-7-35.mp4 \
\
 -i pipe-7-36.mp4 \
\
 -i pipe-7-37.mp4 \
\
 -i pipe-7-38.mp4 \
\
 -i pipe-7-39.mp4 \
\
 -i pipe-8-0.mp4 \
\
 -i pipe-8-1.mp4 \
\
 -i pipe-8-2.mp4 \
\
 -i pipe-8-3.mp4 \
\
 -i pipe-8-4.mp4 \
\
 -i pipe-8-5.mp4 \
\
 -i pipe-8-6.mp4 \
\
 -i pipe-8-7.mp4 \
\
 -i pipe-8-8.mp4 \
\
 -i pipe-8-9.mp4 \
\
 -i pipe-8-10.mp4 \
\
 -i pipe-8-11.mp4 \
\
 -i pipe-8-12.mp4 \
\
 -i pipe-8-13.mp4 \
\
 -i pipe-8-14.mp4 \
\
 -i pipe-8-15.mp4 \
\
 -i pipe-8-16.mp4 \
\
 -i pipe-8-17.mp4 \
\
 -i pipe-8-18.mp4 \
\
 -i pipe-8-19.mp4 \
\
 -i pipe-8-20.mp4 \
\
 -i pipe-8-21.mp4 \
\
 -i pipe-8-22.mp4 \
\
 -i pipe-8-23.mp4 \
\
 -i pipe-8-24.mp4 \
\
 -i pipe-8-25.mp4 \
\
 -i pipe-8-26.mp4 \
\
 -i pipe-8-27.mp4 \
\
 -i pipe-8-28.mp4 \
\
 -i pipe-8-29.mp4 \
\
 -i pipe-8-30.mp4 \
\
 -i pipe-8-31.mp4 \
\
 -i pipe-8-32.mp4 \
\
 -i pipe-8-33.mp4 \
\
 -i pipe-8-34.mp4 \
\
 -i pipe-8-35.mp4 \
\
 -i pipe-8-36.mp4 \
\
 -i pipe-8-37.mp4 \
\
 -i pipe-8-38.mp4 \
\
 -i pipe-8-39.mp4 \
\
 -i pipe-9-0.mp4 \
\
 -i pipe-9-1.mp4 \
\
 -i pipe-9-2.mp4 \
\
 -i pipe-9-3.mp4 \
\
 -i pipe-9-4.mp4 \
\
 -i pipe-9-5.mp4 \
\
 -i pipe-9-6.mp4 \
\
 -i pipe-9-7.mp4 \
\
 -i pipe-9-8.mp4 \
\
 -i pipe-9-9.mp4 \
\
 -i pipe-9-10.mp4 \
\
 -i pipe-9-11.mp4 \
\
 -i pipe-9-12.mp4 \
\
 -i pipe-9-13.mp4 \
\
 -i pipe-9-14.mp4 \
\
 -i pipe-9-15.mp4 \
\
 -i pipe-9-16.mp4 \
\
 -i pipe-9-17.mp4 \
\
 -i pipe-9-18.mp4 \
\
 -i pipe-9-19.mp4 \
\
 -i pipe-9-20.mp4 \
\
 -i pipe-9-21.mp4 \
\
 -i pipe-9-22.mp4 \
\
 -i pipe-9-23.mp4 \
\
 -i pipe-9-24.mp4 \
\
 -i pipe-9-25.mp4 \
\
 -i pipe-9-26.mp4 \
\
 -i pipe-9-27.mp4 \
\
 -i pipe-9-28.mp4 \
\
 -i pipe-9-29.mp4 \
\
 -i pipe-9-30.mp4 \
\
 -i pipe-9-31.mp4 \
\
 -i pipe-9-32.mp4 \
\
 -i pipe-9-33.mp4 \
\
 -i pipe-9-34.mp4 \
\
 -i pipe-9-35.mp4 \
\
 -i pipe-9-36.mp4 \
\
 -i pipe-9-37.mp4 \
\
 -i pipe-9-38.mp4 \
\
 -i pipe-9-39.mp4 \
\
 -i pipe-10-0.mp4 \
\
 -i pipe-10-1.mp4 \
\
 -i pipe-10-2.mp4 \
\
 -i pipe-10-3.mp4 \
\
 -i pipe-10-4.mp4 \
\
 -i pipe-10-5.mp4 \
\
 -i pipe-10-6.mp4 \
\
 -i pipe-10-7.mp4 \
\
 -i pipe-10-8.mp4 \
\
 -i pipe-10-9.mp4 \
\
 -i pipe-10-10.mp4 \
\
 -i pipe-10-11.mp4 \
\
 -i pipe-10-12.mp4 \
\
 -i pipe-10-13.mp4 \
\
 -i pipe-10-14.mp4 \
\
 -i pipe-10-15.mp4 \
\
 -i pipe-10-16.mp4 \
\
 -i pipe-10-17.mp4 \
\
 -i pipe-10-18.mp4 \
\
 -i pipe-10-19.mp4 \
\
 -i pipe-10-20.mp4 \
\
 -i pipe-10-21.mp4 \
\
 -i pipe-10-22.mp4 \
\
 -i pipe-10-23.mp4 \
\
 -i pipe-10-24.mp4 \
\
 -i pipe-10-25.mp4 \
\
 -i pipe-10-26.mp4 \
\
 -i pipe-10-27.mp4 \
\
 -i pipe-10-28.mp4 \
\
 -i pipe-10-29.mp4 \
\
 -i pipe-10-30.mp4 \
\
 -i pipe-10-31.mp4 \
\
 -i pipe-10-32.mp4 \
\
 -i pipe-10-33.mp4 \
\
 -i pipe-10-34.mp4 \
\
 -i pipe-10-35.mp4 \
\
 -i pipe-10-36.mp4 \
\
 -i pipe-10-37.mp4 \
\
 -i pipe-10-38.mp4 \
\
 -i pipe-10-39.mp4 \
\
 -i pipe-11-0.mp4 \
\
 -i pipe-11-1.mp4 \
\
 -i pipe-11-2.mp4 \
\
 -i pipe-11-3.mp4 \
\
 -i pipe-11-4.mp4 \
\
 -i pipe-11-5.mp4 \
\
 -i pipe-11-6.mp4 \
\
 -i pipe-11-7.mp4 \
\
 -i pipe-11-8.mp4 \
\
 -i pipe-11-9.mp4 \
\
 -i pipe-11-10.mp4 \
\
 -i pipe-11-11.mp4 \
\
 -i pipe-11-12.mp4 \
\
 -i pipe-11-13.mp4 \
\
 -i pipe-11-14.mp4 \
\
 -i pipe-11-15.mp4 \
\
 -i pipe-11-16.mp4 \
\
 -i pipe-11-17.mp4 \
\
 -i pipe-11-18.mp4 \
\
 -i pipe-11-19.mp4 \
\
 -i pipe-11-20.mp4 \
\
 -i pipe-11-21.mp4 \
\
 -i pipe-11-22.mp4 \
\
 -i pipe-11-23.mp4 \
\
 -i pipe-11-24.mp4 \
\
 -i pipe-11-25.mp4 \
\
 -i pipe-11-26.mp4 \
\
 -i pipe-11-27.mp4 \
\
 -i pipe-11-28.mp4 \
\
 -i pipe-11-29.mp4 \
\
 -i pipe-11-30.mp4 \
\
 -i pipe-11-31.mp4 \
\
 -i pipe-11-32.mp4 \
\
 -i pipe-11-33.mp4 \
\
 -i pipe-11-34.mp4 \
\
 -i pipe-11-35.mp4 \
\
 -i pipe-11-36.mp4 \
\
 -i pipe-11-37.mp4 \
\
 -i pipe-11-38.mp4 \
\
 -i pipe-11-39.mp4 \
\
 -i pipe-12-0.mp4 \
\
 -i pipe-12-1.mp4 \
\
 -i pipe-12-2.mp4 \
\
 -i pipe-12-3.mp4 \
\
 -i pipe-12-4.mp4 \
\
 -i pipe-12-5.mp4 \
\
 -i pipe-12-6.mp4 \
\
 -i pipe-12-7.mp4 \
\
 -i pipe-12-8.mp4 \
\
 -i pipe-12-9.mp4 \
\
 -i pipe-12-10.mp4 \
\
 -i pipe-12-11.mp4 \
\
 -i pipe-12-12.mp4 \
\
 -i pipe-12-13.mp4 \
\
 -i pipe-12-14.mp4 \
\
 -i pipe-12-15.mp4 \
\
 -i pipe-12-16.mp4 \
\
 -i pipe-12-17.mp4 \
\
 -i pipe-12-18.mp4 \
\
 -i pipe-12-19.mp4 \
\
 -i pipe-12-20.mp4 \
\
 -i pipe-12-21.mp4 \
\
 -i pipe-12-22.mp4 \
\
 -i pipe-12-23.mp4 \
\
 -i pipe-12-24.mp4 \
\
 -i pipe-12-25.mp4 \
\
 -i pipe-12-26.mp4 \
\
 -i pipe-12-27.mp4 \
\
 -i pipe-12-28.mp4 \
\
 -i pipe-12-29.mp4 \
\
 -i pipe-12-30.mp4 \
\
 -i pipe-12-31.mp4 \
\
 -i pipe-12-32.mp4 \
\
 -i pipe-12-33.mp4 \
\
 -i pipe-12-34.mp4 \
\
 -i pipe-12-35.mp4 \
\
 -i pipe-12-36.mp4 \
\
 -i pipe-12-37.mp4 \
\
 -i pipe-12-38.mp4 \
\
 -i pipe-12-39.mp4 \
\
 -i pipe-13-0.mp4 \
\
 -i pipe-13-1.mp4 \
\
 -i pipe-13-2.mp4 \
\
 -i pipe-13-3.mp4 \
\
 -i pipe-13-4.mp4 \
\
 -i pipe-13-5.mp4 \
\
 -i pipe-13-6.mp4 \
\
 -i pipe-13-7.mp4 \
\
 -i pipe-13-8.mp4 \
\
 -i pipe-13-9.mp4 \
\
 -i pipe-13-10.mp4 \
\
 -i pipe-13-11.mp4 \
\
 -i pipe-13-12.mp4 \
\
 -i pipe-13-13.mp4 \
\
 -i pipe-13-14.mp4 \
\
 -i pipe-13-15.mp4 \
\
 -i pipe-13-16.mp4 \
\
 -i pipe-13-17.mp4 \
\
 -i pipe-13-18.mp4 \
\
 -i pipe-13-19.mp4 \
\
 -i pipe-13-20.mp4 \
\
 -i pipe-13-21.mp4 \
\
 -i pipe-13-22.mp4 \
\
 -i pipe-13-23.mp4 \
\
 -i pipe-13-24.mp4 \
\
 -i pipe-13-25.mp4 \
\
 -i pipe-13-26.mp4 \
\
 -i pipe-13-27.mp4 \
\
 -i pipe-13-28.mp4 \
\
 -i pipe-13-29.mp4 \
\
 -i pipe-13-30.mp4 \
\
 -i pipe-13-31.mp4 \
\
 -i pipe-13-32.mp4 \
\
 -i pipe-13-33.mp4 \
\
 -i pipe-13-34.mp4 \
\
 -i pipe-13-35.mp4 \
\
 -i pipe-13-36.mp4 \
\
 -i pipe-13-37.mp4 \
\
 -i pipe-13-38.mp4 \
\
 -i pipe-13-39.mp4 \
\
 -i pipe-14-0.mp4 \
\
 -i pipe-14-1.mp4 \
\
 -i pipe-14-2.mp4 \
\
 -i pipe-14-3.mp4 \
\
 -i pipe-14-4.mp4 \
\
 -i pipe-14-5.mp4 \
\
 -i pipe-14-6.mp4 \
\
 -i pipe-14-7.mp4 \
\
 -i pipe-14-8.mp4 \
\
 -i pipe-14-9.mp4 \
\
 -i pipe-14-10.mp4 \
\
 -i pipe-14-11.mp4 \
\
 -i pipe-14-12.mp4 \
\
 -i pipe-14-13.mp4 \
\
 -i pipe-14-14.mp4 \
\
 -i pipe-14-15.mp4 \
\
 -i pipe-14-16.mp4 \
\
 -i pipe-14-17.mp4 \
\
 -i pipe-14-18.mp4 \
\
 -i pipe-14-19.mp4 \
\
 -i pipe-14-20.mp4 \
\
 -i pipe-14-21.mp4 \
\
 -i pipe-14-22.mp4 \
\
 -i pipe-14-23.mp4 \
\
 -i pipe-14-24.mp4 \
\
 -i pipe-14-25.mp4 \
\
 -i pipe-14-26.mp4 \
\
 -i pipe-14-27.mp4 \
\
 -i pipe-14-28.mp4 \
\
 -i pipe-14-29.mp4 \
\
 -i pipe-14-30.mp4 \
\
 -i pipe-14-31.mp4 \
\
 -i pipe-14-32.mp4 \
\
 -i pipe-14-33.mp4 \
\
 -i pipe-14-34.mp4 \
\
 -i pipe-14-35.mp4 \
\
 -i pipe-14-36.mp4 \
\
 -i pipe-14-37.mp4 \
\
 -i pipe-14-38.mp4 \
\
 -i pipe-14-39.mp4 \
\
 -i pipe-15-0.mp4 \
\
 -i pipe-15-1.mp4 \
\
 -i pipe-15-2.mp4 \
\
 -i pipe-15-3.mp4 \
\
 -i pipe-15-4.mp4 \
\
 -i pipe-15-5.mp4 \
\
 -i pipe-15-6.mp4 \
\
 -i pipe-15-7.mp4 \
\
 -i pipe-15-8.mp4 \
\
 -i pipe-15-9.mp4 \
\
 -i pipe-15-10.mp4 \
\
 -i pipe-15-11.mp4 \
\
 -i pipe-15-12.mp4 \
\
 -i pipe-15-13.mp4 \
\
 -i pipe-15-14.mp4 \
\
 -i pipe-15-15.mp4 \
\
 -i pipe-15-16.mp4 \
\
 -i pipe-15-17.mp4 \
\
 -i pipe-15-18.mp4 \
\
 -i pipe-15-19.mp4 \
\
 -i pipe-15-20.mp4 \
\
 -i pipe-15-21.mp4 \
\
 -i pipe-15-22.mp4 \
\
 -i pipe-15-23.mp4 \
\
 -i pipe-15-24.mp4 \
\
 -i pipe-15-25.mp4 \
\
 -i pipe-15-26.mp4 \
\
 -i pipe-15-27.mp4 \
\
 -i pipe-15-28.mp4 \
\
 -i pipe-15-29.mp4 \
\
 -i pipe-15-30.mp4 \
\
 -i pipe-15-31.mp4 \
\
 -i pipe-15-32.mp4 \
\
 -i pipe-15-33.mp4 \
\
 -i pipe-15-34.mp4 \
\
 -i pipe-15-35.mp4 \
\
 -i pipe-15-36.mp4 \
\
 -i pipe-15-37.mp4 \
\
 -i pipe-15-38.mp4 \
\
 -i pipe-15-39.mp4 \
  -filter_complex "\
    [0:v][1:v][2:v][3:v][4:v][5:v][6:v][7:v][8:v][9:v][10:v][11:v][12:v][13:v][14:v][15:v][16:v][17:v][18:v][19:v][20:v][21:v][22:v][23:v][24:v][25:v][26:v][27:v][28:v][29:v][30:v][31:v][32:v][33:v][34:v][35:v][36:v][37:v][38:v][39:v]concat=n=40[tile0];
    [40:v][41:v][42:v][43:v][44:v][45:v][46:v][47:v][48:v][49:v][50:v][51:v][52:v][53:v][54:v][55:v][56:v][57:v][58:v][59:v][60:v][61:v][62:v][63:v][64:v][65:v][66:v][67:v][68:v][69:v][70:v][71:v][72:v][73:v][74:v][75:v][76:v][77:v][78:v][79:v]concat=n=40[tile1];
    [80:v][81:v][82:v][83:v][84:v][85:v][86:v][87:v][88:v][89:v][90:v][91:v][92:v][93:v][94:v][95:v][96:v][97:v][98:v][99:v][100:v][101:v][102:v][103:v][104:v][105:v][106:v][107:v][108:v][109:v][110:v][111:v][112:v][113:v][114:v][115:v][116:v][117:v][118:v][119:v]concat=n=40[tile2];
    [120:v][121:v][122:v][123:v][124:v][125:v][126:v][127:v][128:v][129:v][130:v][131:v][132:v][133:v][134:v][135:v][136:v][137:v][138:v][139:v][140:v][141:v][142:v][143:v][144:v][145:v][146:v][147:v][148:v][149:v][150:v][151:v][152:v][153:v][154:v][155:v][156:v][157:v][158:v][159:v]concat=n=40[tile3];
    [160:v][161:v][162:v][163:v][164:v][165:v][166:v][167:v][168:v][169:v][170:v][171:v][172:v][173:v][174:v][175:v][176:v][177:v][178:v][179:v][180:v][181:v][182:v][183:v][184:v][185:v][186:v][187:v][188:v][189:v][190:v][191:v][192:v][193:v][194:v][195:v][196:v][197:v][198:v][199:v]concat=n=40[tile4];
    [200:v][201:v][202:v][203:v][204:v][205:v][206:v][207:v][208:v][209:v][210:v][211:v][212:v][213:v][214:v][215:v][216:v][217:v][218:v][219:v][220:v][221:v][222:v][223:v][224:v][225:v][226:v][227:v][228:v][229:v][230:v][231:v][232:v][233:v][234:v][235:v][236:v][237:v][238:v][239:v]concat=n=40[tile5];
    [240:v][241:v][242:v][243:v][244:v][245:v][246:v][247:v][248:v][249:v][250:v][251:v][252:v][253:v][254:v][255:v][256:v][257:v][258:v][259:v][260:v][261:v][262:v][263:v][264:v][265:v][266:v][267:v][268:v][269:v][270:v][271:v][272:v][273:v][274:v][275:v][276:v][277:v][278:v][279:v]concat=n=40[tile6];
    [280:v][281:v][282:v][283:v][284:v][285:v][286:v][287:v][288:v][289:v][290:v][291:v][292:v][293:v][294:v][295:v][296:v][297:v][298:v][299:v][300:v][301:v][302:v][303:v][304:v][305:v][306:v][307:v][308:v][309:v][310:v][311:v][312:v][313:v][314:v][315:v][316:v][317:v][318:v][319:v]concat=n=40[tile7];
    [320:v][321:v][322:v][323:v][324:v][325:v][326:v][327:v][328:v][329:v][330:v][331:v][332:v][333:v][334:v][335:v][336:v][337:v][338:v][339:v][340:v][341:v][342:v][343:v][344:v][345:v][346:v][347:v][348:v][349:v][350:v][351:v][352:v][353:v][354:v][355:v][356:v][357:v][358:v][359:v]concat=n=40[tile8];
    [360:v][361:v][362:v][363:v][364:v][365:v][366:v][367:v][368:v][369:v][370:v][371:v][372:v][373:v][374:v][375:v][376:v][377:v][378:v][379:v][380:v][381:v][382:v][383:v][384:v][385:v][386:v][387:v][388:v][389:v][390:v][391:v][392:v][393:v][394:v][395:v][396:v][397:v][398:v][399:v]concat=n=40[tile9];
    [400:v][401:v][402:v][403:v][404:v][405:v][406:v][407:v][408:v][409:v][410:v][411:v][412:v][413:v][414:v][415:v][416:v][417:v][418:v][419:v][420:v][421:v][422:v][423:v][424:v][425:v][426:v][427:v][428:v][429:v][430:v][431:v][432:v][433:v][434:v][435:v][436:v][437:v][438:v][439:v]concat=n=40[tile10];
    [440:v][441:v][442:v][443:v][444:v][445:v][446:v][447:v][448:v][449:v][450:v][451:v][452:v][453:v][454:v][455:v][456:v][457:v][458:v][459:v][460:v][461:v][462:v][463:v][464:v][465:v][466:v][467:v][468:v][469:v][470:v][471:v][472:v][473:v][474:v][475:v][476:v][477:v][478:v][479:v]concat=n=40[tile11];
    [480:v][481:v][482:v][483:v][484:v][485:v][486:v][487:v][488:v][489:v][490:v][491:v][492:v][493:v][494:v][495:v][496:v][497:v][498:v][499:v][500:v][501:v][502:v][503:v][504:v][505:v][506:v][507:v][508:v][509:v][510:v][511:v][512:v][513:v][514:v][515:v][516:v][517:v][518:v][519:v]concat=n=40[tile12];
    [520:v][521:v][522:v][523:v][524:v][525:v][526:v][527:v][528:v][529:v][530:v][531:v][532:v][533:v][534:v][535:v][536:v][537:v][538:v][539:v][540:v][541:v][542:v][543:v][544:v][545:v][546:v][547:v][548:v][549:v][550:v][551:v][552:v][553:v][554:v][555:v][556:v][557:v][558:v][559:v]concat=n=40[tile13];
    [560:v][561:v][562:v][563:v][564:v][565:v][566:v][567:v][568:v][569:v][570:v][571:v][572:v][573:v][574:v][575:v][576:v][577:v][578:v][579:v][580:v][581:v][582:v][583:v][584:v][585:v][586:v][587:v][588:v][589:v][590:v][591:v][592:v][593:v][594:v][595:v][596:v][597:v][598:v][599:v]concat=n=40[tile14];
    [600:v][601:v][602:v][603:v][604:v][605:v][606:v][607:v][608:v][609:v][610:v][611:v][612:v][613:v][614:v][615:v][616:v][617:v][618:v][619:v][620:v][621:v][622:v][623:v][624:v][625:v][626:v][627:v][628:v][629:v][630:v][631:v][632:v][633:v][634:v][635:v][636:v][637:v][638:v][639:v]concat=n=40[tile15];
    [tile0][tile1][tile2][tile3]hstack=inputs=4[row0];
    [tile4][tile5][tile6][tile7]hstack=inputs=4[row1];
    [tile8][tile9][tile10][tile11]hstack=inputs=4[row2];
    [tile12][tile13][tile14][tile15]hstack=inputs=4[row3];
    [row0][row1][row2][row3]vstack=inputs=4[out]
  " \
  -map "[out]" \
    -f segment \
      -vcodec hevc_nvenc \
      -b:v 5000k \
      -minrate 5000k \
      -maxrate 5000k \
      -r 30 -g 30 \
      -segment_format mp4 \
      -segment_time_delta 0.05 \
      -segment_time 1 \
    output-%d.mp4
rm pipe*
du -ch output-* | tail -n 1
rm output-*
