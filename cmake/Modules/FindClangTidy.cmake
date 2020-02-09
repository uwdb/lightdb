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
  string(REPLACE ";" ";-extra-arg-before;-I" CLANG_TIDY_INCLUDE_DIRS "${CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_INCLUDE_DIRS}")

  if(NOT DEFINED CLANG_TIDY_CXX_STANDARD)
    set(CLANG_TIDY_CXX_STANDARD ${CMAKE_CXX_STANDARD})
  endif()

  foreach (source ${CLANG_TIDY_EXCLUDED_SOURCES})
    set(CLANG_TIDY_LINE_FILTERS ",{\"name\":\"${source}\",\"lines\":[[1,1]]}${CLANG_TIDY_LINE_FILTERS}")
  endforeach()
  string(SUBSTRING "${CLANG_TIDY_LINE_FILTERS}" 1 -1 CLANG_TIDY_LINE_FILTERS)

  set(EXECUTE_CLANGTIDY "${CLANGTIDY_PATH}" "-checks=*,-clang-analyzer-alpha.*;"
                                            "-extra-arg;-std=c++${CLANG_TIDY_CXX_STANDARD};"
                                            "-extra-arg-before;-I${NVIDIASDK_INCLUDE_DIR};"
                                            "-extra-arg-before;-I${CLANG_TIDY_INCLUDE_DIRS};"
                                            "-line-filter;[${CLANG_TIDY_LINE_FILTERS}]")

  set(CMAKE_CXX_CLANG_TIDY ${EXECUTE_CLANGTIDY})
  set(CLANGTIDY_FOUND TRUE)
endif()

if(CLANGTIDY_FIND_REQUIRED AND NOT CLANGTIDY_FOUND)
  message(FATAL_ERROR "Could not find the clang-tidy utility.")
endif()

