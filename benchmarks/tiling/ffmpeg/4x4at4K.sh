# height: 2160, width: 3840
# rows: 4, columns: 4
#tile width: 960, tile height: 540
ffmpeg \
  -i /home/bhaynes/projects/visualcloud/test/resources/test-pattern-2K.h264 \
  -vcodec h264_cuvid \
  -filter_complex "
    [0:v]crop=960:540:0:0[tile0];
    [0:v]crop=960:540:960:0[tile1];
    [0:v]crop=960:540:1920:0[tile2];
    [0:v]crop=960:540:2880:0[tile3];
    [0:v]crop=960:540:0:540[tile4];
    [0:v]crop=960:540:960:540[tile5];
    [0:v]crop=960:540:1920:540[tile6];
    [0:v]crop=960:540:2880:540[tile7];
    [0:v]crop=960:540:0:1080[tile8];
    [0:v]crop=960:540:960:1080[tile9];
    [0:v]crop=960:540:1920:1080[tile10];
    [0:v]crop=960:540:2880:1080[tile11];
    [0:v]crop=960:540:0:1620[tile12];
    [0:v]crop=960:540:960:1620[tile13];
    [0:v]crop=960:540:1920:1620[tile14];
    [0:v]crop=960:540:2880:1620[tile15]" \
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
  -r 30 \
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
  -filter_complex "\
    [0:v][1:v][2:v][3:v][4:v][5:v][6:v][7:v][8:v][9:v][10:v][11:v][12:v][13:v][14:v][15:v][16:v][17:v][18:v][19:v]concat=n=20[tile0];
    [20:v][21:v][22:v][23:v][24:v][25:v][26:v][27:v][28:v][29:v][30:v][31:v][32:v][33:v][34:v][35:v][36:v][37:v][38:v][39:v]concat=n=20[tile1];
    [40:v][41:v][42:v][43:v][44:v][45:v][46:v][47:v][48:v][49:v][50:v][51:v][52:v][53:v][54:v][55:v][56:v][57:v][58:v][59:v]concat=n=20[tile2];
    [60:v][61:v][62:v][63:v][64:v][65:v][66:v][67:v][68:v][69:v][70:v][71:v][72:v][73:v][74:v][75:v][76:v][77:v][78:v][79:v]concat=n=20[tile3];
    [80:v][81:v][82:v][83:v][84:v][85:v][86:v][87:v][88:v][89:v][90:v][91:v][92:v][93:v][94:v][95:v][96:v][97:v][98:v][99:v]concat=n=20[tile4];
    [100:v][101:v][102:v][103:v][104:v][105:v][106:v][107:v][108:v][109:v][110:v][111:v][112:v][113:v][114:v][115:v][116:v][117:v][118:v][119:v]concat=n=20[tile5];
    [120:v][121:v][122:v][123:v][124:v][125:v][126:v][127:v][128:v][129:v][130:v][131:v][132:v][133:v][134:v][135:v][136:v][137:v][138:v][139:v]concat=n=20[tile6];
    [140:v][141:v][142:v][143:v][144:v][145:v][146:v][147:v][148:v][149:v][150:v][151:v][152:v][153:v][154:v][155:v][156:v][157:v][158:v][159:v]concat=n=20[tile7];
    [160:v][161:v][162:v][163:v][164:v][165:v][166:v][167:v][168:v][169:v][170:v][171:v][172:v][173:v][174:v][175:v][176:v][177:v][178:v][179:v]concat=n=20[tile8];
    [180:v][181:v][182:v][183:v][184:v][185:v][186:v][187:v][188:v][189:v][190:v][191:v][192:v][193:v][194:v][195:v][196:v][197:v][198:v][199:v]concat=n=20[tile9];
    [200:v][201:v][202:v][203:v][204:v][205:v][206:v][207:v][208:v][209:v][210:v][211:v][212:v][213:v][214:v][215:v][216:v][217:v][218:v][219:v]concat=n=20[tile10];
    [220:v][221:v][222:v][223:v][224:v][225:v][226:v][227:v][228:v][229:v][230:v][231:v][232:v][233:v][234:v][235:v][236:v][237:v][238:v][239:v]concat=n=20[tile11];
    [240:v][241:v][242:v][243:v][244:v][245:v][246:v][247:v][248:v][249:v][250:v][251:v][252:v][253:v][254:v][255:v][256:v][257:v][258:v][259:v]concat=n=20[tile12];
    [260:v][261:v][262:v][263:v][264:v][265:v][266:v][267:v][268:v][269:v][270:v][271:v][272:v][273:v][274:v][275:v][276:v][277:v][278:v][279:v]concat=n=20[tile13];
    [280:v][281:v][282:v][283:v][284:v][285:v][286:v][287:v][288:v][289:v][290:v][291:v][292:v][293:v][294:v][295:v][296:v][297:v][298:v][299:v]concat=n=20[tile14];
    [300:v][301:v][302:v][303:v][304:v][305:v][306:v][307:v][308:v][309:v][310:v][311:v][312:v][313:v][314:v][315:v][316:v][317:v][318:v][319:v]concat=n=20[tile15];
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
