# Include core header directories
file(GLOB LIGHTDB_INCLUDE_DIRS "*/include")
include_directories(${LIGHTDB_INCLUDE_DIRS})

file(GLOB LIGHTDB_SOURCE_DIRS "*/src")
foreach(subdir ${LIGHTDB_SOURCE_DIRS})
  add_subdirectory(${subdir}/..)
endforeach()

# Gather the core source files
file(GLOB_RECURSE LIGHTDB_SOURCES "*.cc")
set(LIGHTDB_SOURCES ${LIGHTDB_SOURCES} ${NVIDIASDK_SOURCES})

# Compile sources with PIC
add_library(lightdb OBJECT ${LIGHTDB_SOURCES})
set_property(TARGET lightdb PROPERTY POSITION_INDEPENDENT_CODE ON)

# Create static library
add_library(lightdb_static STATIC $<TARGET_OBJECTS:lightdb>)
set_target_properties(lightdb_static PROPERTIES OUTPUT_NAME "lightdb")

# Create dynamic library
add_library(lightdb_shared SHARED $<TARGET_OBJECTS:lightdb>)
set_target_properties(lightdb_shared PROPERTIES OUTPUT_NAME "lightdb")
target_link_libraries(lightdb_shared ${LIGHTDB_LIB_DEPENDENCIES})

# Install libraries
install(
    TARGETS lightdb_shared lightdb_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
file(GLOB_RECURSE LIGHTDB_HEADERS "include/*.h")
install(FILES ${LIGHTDB_HEADERS} DESTINATION include)

