cmake_minimum_required(VERSION 3.8)
project(pythonmodules)

set(CMAKE_CXX_STANDARD 17)

find_package(Python2 COMPONENTS Development)

# Include core header directories
file(GLOB LIGHTDB_CORE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/../core/*/include/")
include_directories(${LIGHTDB_CORE_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${Python2_INCLUDE_DIRS})

# Include ipp
FIND_PACKAGE (IPP REQUIRED )
include_directories ( ${IPP_INCLUDE_DIRS} )
link_directories ( ${IPP_LIBRARIES} )

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


target_link_libraries(pylightdb ${Boost_LIBRARIES} ${Python2_LIBRARIES} lightdb_shared)
target_link_libraries(pylightdb opencv_core ${IPP_LIB_I} ${IPP_LIB_CC} boost_numpy)
set_target_properties(pylightdb PROPERTIES PREFIX "")
