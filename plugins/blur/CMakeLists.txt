cmake_minimum_required(VERSION 3.8)
project(blur)

set(CMAKE_CXX_STANDARD 17)

include_directories(/opt/intel/ipp/include)

include_directories(${LIGHTDB_INCLUDES})

find_package(CUDA 7.5 REQUIRED)
set(BLUR_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} ippi ippcc) #${CUDA_nppi_LIBRARY})

link_directories(/opt/intel/ipp/lib/intel64)

add_library(blur SHARED udf.cc udf.h)

target_link_libraries(blur ${BLUR_LIB_DEPENDENCIES})

ADD_CUSTOM_COMMAND(TARGET blur
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:blur> ${CMAKE_INSTALL_PREFIX}
        )
