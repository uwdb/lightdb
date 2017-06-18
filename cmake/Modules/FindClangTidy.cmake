# Finds the clang-tidy utility. This module defines:
#   - CLANGTIDY_PATH, the utility path
#   - CLANGTIDY_FOUND, whether clang-tidy has been found

find_program(
  CLANGTIDY_PATH
  NAMES "clang-tidy"
  )

if(NOT CLANGTIDY_PATH)
  set(CLANGTIDY_FOUND FALSE)
else()
  message(STATUS "Found clang-tidy: ${CLANGTIDY_PATH}")
  set(EXECUTE_CLANGTIDY "${CLANGTIDY_PATH}" "-checks=*,-clang-analyzer-alpha.*")
  set(CMAKE_CXX_CLANG_TIDY ${EXECUTE_CLANGTIDY})
  set(CLANGTIDY_FOUND TRUE)
endif()

if(CLANGTIDY_FIND_REQUIRED AND NOT CLANGTIDY_FOUND)
  message(FATAL_ERROR "Could not find the clang-tidy utility.")
endif()

