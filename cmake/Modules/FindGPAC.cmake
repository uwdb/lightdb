# - Find GPAC
# Find the native gpac includes and library.
# Once done this will define
#
#  GPAC_INCLUDE_DIRS   - where to find GPAC header files.
#  GPAC_LIBRARIES      - List of libraries when using GPAC.
#  GPAC_FOUND          - True if GPAC found.
#
# https://sourceforge.net/p/graphicscodec/repository/70/tree/trunk/cmakemodules/FindGPAC.cmake

find_path(GPAC_INCLUDE_DIR gpac/isomedia.h)

find_library(GPAC_LIBRARY NAMES gpac)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set GPAC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GPAC  DEFAULT_MSG
        GPAC_LIBRARY GPAC_INCLUDE_DIR)

mark_as_advanced(GPAC_INCLUDE_DIR GPAC_LIBRARY )

if(GPAC_FOUND)
  set(GPAC_INCLUDE_DIRS ${GPAC_INCLUDE_DIR})
  set(GPAC_LIBRARIES "/usr/lib/x86_64-linux-gnu/libgpac.so")
#  set(GPAC_LIBRARIES ${GPAC_LIBRARY})
endif()