# Downloads and finds the Bento4 library. This module defines:
#   - Bento4_SOURCE_DIR
#   - Bento4_INCLUDE_DIRS
#   - Bento4_BINARY_DIR
#   - Bento4_FOUND, whether Bento4 has been downloaded successfully

find_program(
  Bento4_PATH
  NAMES "Bento4"
  )

download_project(
        PROJ                Bento4
        GIT_REPOSITORY      https://github.com/axiomatic-systems/Bento4
        GIT_TAG             v1.5.1-624
        UPDATE_DISCONNECTED 1
        QUIET
)

set(Bento4_FOUND TRUE)
file(GLOB Bento4_INCLUDE_DIRS "${Bento4_SOURCE_DIR}/Source/C++/*")

if(Bento4_FIND_REQUIRED AND NOT Bento4_FOUND)
  message(FATAL_ERROR "Could not find the Bento4 library.")
endif()

