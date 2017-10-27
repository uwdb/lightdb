#!/usr/bin/env python

from sys import argv

def split_command(height, width, rows, columns, bitrates):
#  -r 30 -g 30 \\
    return """ffmpeg \\
  -i {input_filename} \\
  -vcodec h264_cuvid \\
  -filter_complex "{filter}" \\
{maps}
  """.format(input_filename=input_filename,
             filter=create_filter(height, width, rows, columns),
             maps=create_maps(rows, columns, bitrates))

def stitch_command(rows, columns, height, width, fps, duration, output_format, output_segment_duration):
    command = """ffmpeg \\
  -y \\
  -r {fps} \\\n""".format(fps=fps)

    for i in xrange(rows*columns):
      for s in xrange(duration):
        command += "\\\n -i pipe-{}-{}.mp4 \\\n".format(i, s)

    command += "  -filter_complex \"\\\n"

    for i in xrange(rows*columns):
      command += "    "
      for s in xrange(duration):
        command += "[{}:v]".format(i * duration + s)
      command += "concat=n={}[tile{}];\n".format(duration, i)

    for row in xrange(rows):
      command += "    "
      for col in xrange(columns):
        command += "[tile{}]".format(row*columns + col)
      command += "hstack=inputs={}[row{}];\n".format(columns, row)

    command += "    "
    for row in xrange(rows):
      command += "[row{}]".format(row)
    command += "vstack=inputs={}[out]\n".format(rows)

    command += '  " \\\n'

    command += """  -map "[out]" \\
    -f segment \\
      -vcodec hevc_nvenc \\
      -b:v 5000k \\
      -minrate 5000k \\
      -maxrate 5000k \\
      -r 30 -g 30 \\
      -segment_format mp4 \\
      -segment_time_delta 0.05 \\
      -segment_time {} \\
    output-%d.mp4""".format(output_segment_duration)

    return command


def create_filter(height, width, rows, columns):
  filter = ""
  tile_height, tile_width = height / rows, width / columns

  print "#tile width: {}, tile height: {}".format(tile_width, tile_height)

  for row in xrange(rows):
    for column in xrange(columns):
      i = column + columns*row
      offset_x, offset_y = column * tile_width, row * tile_height
      filter += "\n    [0:v]crop={}:{}:{}:{}[tile{}];".format(tile_width, tile_height, offset_x, offset_y, i)

  return filter[:-1]

def create_maps(rows, columns, bitrates):
  maps = ""
  for i in xrange(rows * columns):
    maps += """  -map "[tile{index}]" \\
    -vcodec h264_nvenc \\
    -b:v {bitrate}k \\
    -minrate {bitrate}k \\
    -maxrate {bitrate}k \\
    -r 30 -g 30 \\
    -f segment \\
    -segment_format mp4 \\
    -segment_format_options movflags=+faststart+frag_keyframe+default_base_moof \\
    -segment_time 1 \\
    -segment_time_delta 0.05 \\
    pipe-{index}-%d.mp4 \\\n""".format(index=i, bitrate=bitrates[i])
  return maps

if __name__ == "__main__":
  if len(argv) != 8:
    print "Usage: {} [input filename] [height] [width] [rows] [columns] [duration] [output fps]".format(argv[0])
  else:
    input_filename = argv[1]
    height, width, rows, columns, duration, fps = map(int, argv[2:])

    bitrates = [1000, 5000, 1000] + [50] * (rows * columns - 3)
    #bitrates = [50, 50, 50, 50, 50, 50, 1000, 5000, 1000, 50, 50, 50, 50, 50, 50]
    output_format, output_segment_duration = 'hevc', 1

    print "# height: {}, width: {}".format(height, width)
    print "# rows: {}, columns: {}".format(rows, columns)

    print split_command(height, width, rows, columns, bitrates)
    print
    print stitch_command(rows, columns, height, width, fps, duration, output_format, output_segment_duration)
