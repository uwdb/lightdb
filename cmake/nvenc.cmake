set(NVENC_ROOT thirdparty/nv-codec-headers)

include_directories(SYSTEM
    ${PROJECT_SOURCE_DIR}/${NVENC_ROOT}
    ${PROJECT_SOURCE_DIR}/${NVENC_ROOT}/inc
    )

#add_library(nvenc ${NVENC_SOURCES})
