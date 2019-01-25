cmake_minimum_required(VERSION 3.8)
project(blur)

set(CMAKE_CXX_STANDARD 17)

include_directories(/home/bhaynes/projects/visualcloud/core/model/include)
include_directories(/home/bhaynes/projects/visualcloud/core/catalog/include)
include_directories(/home/bhaynes/projects/visualcloud/core/video/include)
include_directories(/home/bhaynes/projects/visualcloud/core/physical/include)
include_directories(/home/bhaynes/projects/visualcloud/core/utility/include)
include_directories(/home/bhaynes/projects/visualcloud/core/pool/include)
include_directories(/opt/intel/compilers_and_libraries_2018.3.222/linux/ipp/include)
include_directories(/opt/nvidia/Video_Codec_SDK_7.1.9/Samples/common/inc)
include_directories(/usr/local/cuda-8.0/include)

link_directories(/home/bhaynes/projects/visualcloud/core)
link_directories(/opt/intel/ipp/lib/intel64)
set(BLUR_LIB_DEPENDENCIES ${BLUR_LIB_DEPENDENCIES} npps nppi)
set(BLUR_LIB_DEPENDENCIES ${YOLO_LIB_DEPENDENCIES} ipps ippi ippcc)

add_library(blur SHARED udf.cc udf.h)

target_link_libraries(blur ${BLUR_LIB_DEPENDENCIES})

ADD_CUSTOM_COMMAND(TARGET blur
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:blur> ${PROJECT_SOURCE_DIR}/..
        )
