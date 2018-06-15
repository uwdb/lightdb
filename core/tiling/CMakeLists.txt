cmake_minimum_required (VERSION 2.8)
project(LightDB)

cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)

find_package(GTest)
enable_testing()

file(GLOB_RECURSE LIGHTDB_SOURCES "src/*.cpp" "include/*.h")
set(LIGHTDB_LIB_DEPENDENCIES glog)

add_subdirectory(src)
add_subdirectory(include)
include_directories(include)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(LightDB ${LIGHTDB_SOURCES})
target_link_libraries(${PROJECT_NAME} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${LIGHTDB_LIB_DEPENDENCIES})