cmake_minimum_required(VERSION 3.8)
project(yolo)

set(CMAKE_CXX_STANDARD 17)
include(ExternalProject)

set(DARKNET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/darknet)

ExternalProject_Add(darknet
        PREFIX darknet
        GIT_REPOSITORY "https://github.com/pjreddie/darknet.git"
        GIT_TAG "c7252703420159a9f3a1ec416b1b4326c4c95402"
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        INSTALL_COMMAND ""
        SOURCE_DIR ${DARKNET_DIR}
        INSTALL_DIR ${DARKNET_DIR}
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND make CFLAGS=-fPIC -w
        BUILD_BYPRODUCTS ${DARKNET_DIR}/libdarknet.so)
add_library(libdarknet SHARED IMPORTED)
set_target_properties(libdarknet PROPERTIES IMPORTED_LOCATION ${DARKNET_DIR}/libdarknet.so)

include_directories(/opt/intel/ipp/include)
include_directories(${LIGHTDB_INCLUDES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/darknet/include)

link_directories(/opt/intel/ipp/lib/intel64)
set(YOLO_LIB_DEPENDENCIES ${YOLO_LIB_DEPENDENCIES} ipps ippi ippcc)

add_library(yolo SHARED udf.cc udf.h)

add_dependencies(yolo libdarknet)

target_link_libraries(yolo ${YOLO_LIB_DEPENDENCIES})

ADD_CUSTOM_COMMAND(TARGET yolo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:yolo> ${CMAKE_INSTALL_PREFIX}
        )
