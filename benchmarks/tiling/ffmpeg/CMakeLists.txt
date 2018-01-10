cmake_minimum_required(VERSION 3.8)
project(ffmpeg_tiler)

set(CMAKE_CXX_STANDARD 14)

include_directories("/usr/include/ffmpeg")

set(SOURCE_FILES tile.cc)
add_executable(ffmpeg_tiler ${SOURCE_FILES})
link_directories(/usr/lib/ffmpeg)
target_link_libraries(ffmpeg_tiler avcodec swscale avutil avformat darknet)
