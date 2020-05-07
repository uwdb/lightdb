cmake_minimum_required(VERSION 3.8)
project(pythonmodules)

set(CMAKE_CXX_STANDARD 17)

#set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
#set(CMAKE_CXX_FLAGS "-fvisibility=default")

# Set the cmake Module path
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")

find_package(Python2 COMPONENTS Development)
message("python2 libraries: ${Python2_LIBRARIES}")
message("python2 includes: ${Python2_INCLUDE_DIRS}")

# Include core header directories
file(GLOB LIGHTDB_CORE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/../core/*/include/")
message("includes: ${Boost_INCLUDE_DIRS}")
include_directories(${LIGHTDB_CORE_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${Python2_INCLUDE_DIRS})

# Build Python modules.
set(ALGEBRA_SOURCES "src/Algebra_ext.cc")
set(OPTIONS_SOURCES "src/options.cc")
add_library(pylightdb SHARED ${ALGEBRA_SOURCES} ${OPTIONS_SOURCES})
target_compile_options(pylightdb PRIVATE "-fvisibility=default")

message("boost libraries: ${Boost_LIBRARIES}")

target_link_libraries(pylightdb ${Boost_LIBRARIES} ${Python2_LIBRARIES} ${LIGHTDB_LIB_DEPENDENCIES} lightdb_protobuf lightdb)
target_link_libraries(pylightdb opencv_core)
set_target_properties(pylightdb PROPERTIES PREFIX "")
