# Include core header directories
file(GLOB VISUALCLOUD_INCLUDE_DIRS "*/include")
include_directories(${VISUALCLOUD_INCLUDE_DIRS})

file(GLOB VISUALCLOUD_SOURCE_DIRS "*/src")
foreach(subdir ${VISUALCLOUD_SOURCE_DIRS})
  add_subdirectory(${subdir}/..)
endforeach()

#add_subdirectory("video")

# Gather the core source files
file(GLOB_RECURSE VISUALCLOUD_SOURCES "*.cc")
set(VISUALCLOUD_SOURCES ${VISUALCLOUD_SOURCES} ${NVIDIASDK_SOURCES} video/include/DecoderLock.h)

# Compile sources with PIC
add_library(visualcloud OBJECT ${VISUALCLOUD_SOURCES})
set_property(TARGET visualcloud PROPERTY POSITION_INDEPENDENT_CODE ON)

# Create static library
add_library(visualcloud_static STATIC $<TARGET_OBJECTS:visualcloud>)
set_target_properties(visualcloud_static PROPERTIES OUTPUT_NAME "visualcloud")

# Create dynamic library
add_library(visualcloud_shared SHARED $<TARGET_OBJECTS:visualcloud>)
set_target_properties(visualcloud_shared PROPERTIES OUTPUT_NAME "visualcloud")
target_link_libraries(visualcloud_shared ${VISUALCLOUD_LIB_DEPENDENCIES})

# Install libraries
install(
    TARGETS visualcloud_shared visualcloud_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
file(GLOB_RECURSE VISUALCLOUD_HEADERS "include/*.h")
install(FILES ${VISUALCLOUD_HEADERS} DESTINATION include)

