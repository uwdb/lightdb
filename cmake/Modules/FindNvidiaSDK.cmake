# Finds the nvidia-encode library. This module defines:
#   - NVIDIASDK_INCLUDE_DIR, include file path
#   - NVIDIASDK_SOURCE_DIR, dynamic link source path
#   - NVIDIASDK_FOUND, whether the Nvidia SDK has been found

if(NVIDIASDK_SEARCH_HEADER_PATHS)
  find_path(
      NVIDIASDK_INCLUDE_DIR dynlink_cuda.h
      PATHS ${NVIDIASDK_SEARCH_HEADER_PATHS}
      NO_DEFAULT_PATH)
  find_path(
      NVIDIASDK_SOURCE_DIR dynlink_cuda.cpp
      PATHS ${NVIDIASDK_SEARCH_HEADER_PATHS}
      NO_DEFAULT_PATH)
else()
  find_path(NVIDIASDK_INCLUDE_DIR dynlink_cuda.h
            PATHS file(GLOB /opt/nvidia/Video_Codec_SDK_7.1.9/Samples/common/inc))
  find_path(NVIDIASDK_SOURCE_DIR dynlink_cuda.cpp
            PATHS file(GLOB /opt/nvidia/Video_Codec_SDK_7.1.9/Samples/common/src))
endif()

if(NVIDIASDK_INCLUDE_DIR)
  message(STATUS "Found nvidia-sdk: ${NVIDIASDK_INCLUDE_DIR}")
  set(NVIDIASDK_SOURCES "${NVIDIASDK_SOURCE_DIR}/dynlink_cuda.cpp;${NVIDIASDK_SOURCE_DIR}/dynlink_nvcuvid.cpp")
  set(NVIDIASDK_FOUND TRUE)
else()
  set(NVIDIASDK_FOUND FALSE)
endif()

if(NvidiaSDK_FIND_REQUIRED AND NOT NVIDIASDK_FOUND)
  message(FATAL_ERROR "Could not find the Nvidia SDK.")
endif()

