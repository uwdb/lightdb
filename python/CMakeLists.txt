cmake_minimum_required(VERSION 3.8)
project(pythonmodules)

set(CMAKE_CXX_STANDARD 17)

find_package(Python2 COMPONENTS Development)
message("python2 libraries: ${Python2_LIBRARIES}")
message("python2 includes: ${Python2_INCLUDE_DIRS}")

# Include core header directories
file(GLOB LIGHTDB_CORE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/../core/*/include/")
message("includes: ${Boost_INCLUDE_DIRS}")
include_directories(${LIGHTDB_CORE_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${Python2_INCLUDE_DIRS})

# Include ipp
include_directories(/opt/intel/ipp/include)
link_directories(/opt/intel/ipp/lib/intel64)

# Include python header files
file(GLOB LIGHTDB_PYTHON_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/../python/include")
include_directories(${LIGHTDB_PYTHON_INCLUDE_DIRS})

file(GLOB LIGHTDB_PYTHON_SOURCE_DIRS "*/src")
foreach(subdir ${LIGHTDB_PYTHON_SOURCE_DIRS})
  add_subdirectory(${subdir}/..)
endforeach()

# Gather the core source files
file(GLOB_RECURSE LIGHTDB_PYTHON_SOURCES "*.cc")
set(LIGHTDB_SOURCES ${LIGHTDB_PYTHON_SOURCES})


# Build Python modules.
add_library(pylightdb SHARED ${LIGHTDB_PYTHON_SOURCES})
target_compile_options(pylightdb PRIVATE "-fvisibility=default")

message("boost libraries: ${Boost_LIBRARIES}")

target_link_libraries(pylightdb ${Boost_LIBRARIES} ${Python2_LIBRARIES} lightdb_shared)
#target_link_libraries(pylightdb ${Boost_LIBRARIES} ${Python2_LIBRARIES} ${LIGHTDB_LIB_DEPENDENCIES} lightdb_protobuf lightdb_shared)
target_link_libraries(pylightdb opencv_core ippi ippcc)
set_target_properties(pylightdb PROPERTIES PREFIX "")
