# Finds the nvidia-encode library. This module defines:
#   - NVENC_LIBRARIES, the library path
#   - NVENC_FOUND, whether nvenc has been found

if(NVENC_SEARCH_LIB_PATH)
  find_library(
      NVENC_LIBRARIES NAMES nvidia-encode
      PATHS ${NVENC_SEARCH_LIB_PATH}
      NO_DEFAULT_PATH
  )
else()
  find_library(NVENC_LIBRARIES
               NAMES nvidia-encode)
endif()

if(NVENC_LIBRARIES)
  message(STATUS "Found nvidia-encode: ${NVENC_LIBRARIES}")
  set(NVENC_FOUND TRUE)
else()
  set(NVENC_FOUND FALSE)
endif()

if(NVENC_FIND_REQUIRED AND NOT NVENC_FOUND)
  message(FATAL_ERROR "Could not find the nvidia-encode library.")
endif()

