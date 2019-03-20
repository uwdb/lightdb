project(LightDB)
cmake_minimum_required(VERSION 3.8)

SET(BOOST_MIN_VERSION "1.64.0")

# Set the cmake Module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


# Set shared library suffix
if(APPLE)
  set(SHARED_LIB_SUFFIX ".dylib")
else()
  set(SHARED_LIB_SUFFIX ".so")
endif()

# Only for Mac OS X warnings
if(APPLE)
  set(CMAKE_MACOSX_RPATH 1)
endif()

# Default user definitions
set(GTEST_FILTER "*" CACHE STRING "Filters for the Google unit tests.")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set C++ standards
set (CMAKE_CXX_STANDARD 17)
#set (CLANG_TIDY_CXX_STANDARD 14)

# Set compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g3 -rdynamic -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -fvisibility=hidden -Wall  -Wextra -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_COVERAGE "-DDEBUG --coverage -g3 -Wall -Wextra -Wno-unused-parameter")

# Find required library dependencies
set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} cuda nvcuvid dl pthread glog gflags stdc++fs)

include(cmake/nvenc.cmake)

find_package(CUDA 7.5 REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})
#set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} ${CUDA_nppi_LIBRARY})
set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} ${CUDA_nvcuvenc_LIBRARY})

#TODO add FindLLV.cmake; clean this up
#TODO Do we still depend on OpenCV outside of plugins?
#find_package(OpenCV REQUIRED)
#set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} ${OPENCV_LIBS} opencv_features2d)
#set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} lfv)

#find_package(NVENC REQUIRED)
#set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} ${NVENC_LIBRARIES})

#find_package(NvidiaSDK REQUIRED)
#include_directories(SYSTEM ${NVIDIASDK_INCLUDE_DIR})
#set(CLANG_TIDY_EXCLUDED_SOURCES ${NVIDIASDK_SOURCES})

find_package(PythonLibs 2.7 REQUIRED)
include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS})

find_package(Boost 1.64 COMPONENTS python filesystem system REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} ${Boost_LIBRARIES})

find_package(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE)
include_directories(SYSTEM ${FFMPEG_INCLUDE_DIRS})

set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} ${FFMPEG_LIBRARIES})

find_package (Glog REQUIRED)
#find_package(ClangTidy)

#find_package(Bento4 REQUIRED)
#include_directories(${Bento4_INCLUDE_DIRS})
#add_subdirectory(${Bento4_SOURCE_DIR})

#TODO Add FindGPAC.cmake
set(LIGHTDB_LIB_DEPENDENCIES ${LIGHTDB_LIB_DEPENDENCIES} gpac)
include_directories(SYSTEM /usr/local/include/gpac)

# Build LightDB library
add_subdirectory(core)

# Enable testing
option(ENABLE_TESTS "Enable tests" ON)
if (${ENABLE_TESTS})
  enable_testing()
  add_subdirectory(test)
endif()

# Add clang-format target
file(GLOB_RECURSE FORMATTED_SOURCE_FILES *.cc *.h)
add_custom_target(
  clang-format
  COMMAND /usr/bin/clang-format
  -style=file
  -i
  ${FORMATTED_SOURCE_FILES}
)

# Uninstall
set(CMD "xargs printf '-- Uninstalling: %s\\\\n' <install_manifest.txt")
add_custom_target(
   uninstall
   COMMAND eval "${CMD}"
   COMMAND xargs rm -f < install_manifest.txt
   COMMAND echo "LightDB uninstalled"
)
