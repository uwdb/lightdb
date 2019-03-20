set(NVENC_ROOT thirdparty/nvenc CACHE STRING "NvEnc source root")

include_directories(SYSTEM
    ${PROJECT_SOURCE_DIR}/${NVENC_ROOT}
    ${PROJECT_SOURCE_DIR}/${NVENC_ROOT}/inc
    )

#add_library(nvenc ${NVENC_SOURCES})
