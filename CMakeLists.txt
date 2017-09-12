project(VisualCloud)
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

# Find required library dependencies
set(VISUALCLOUD_LIB_DEPENDENCIES ${VISUALCLOUD_LIB_DEPENDENCIES} cuda nvcuvid dl pthread glog)

find_package(NVENC REQUIRED)
set(VISUALCLOUD_LIB_DEPENDENCIES ${VISUALCLOUD_LIB_DEPENDENCIES} ${NVENC_LIBRARIES})

find_package(NvidiaSDK REQUIRED)
include_directories(${NVIDIASDK_INCLUDE_DIR})

find_package(PythonLibs 2.7 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

find_package(Boost COMPONENTS python REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

find_package(GTest)
find_package (Glog REQUIRED)
find_package(ClangTidy)

# Set compiler flags
set (CMAKE_CXX_STANDARD 17)

# Set compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g3 -rdynamic -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -fvisibility=hidden -Wall")
set(CMAKE_CXX_FLAGS_COVERAGE "-DDEBUG --coverage -g3 -Wall")

# Enable testing
enable_testing()

# Build VisualCloud library
add_subdirectory(core)

# Build unit tests
if(GTEST_FOUND)
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
   COMMAND echo "VisualCloud uninstalled"
)
