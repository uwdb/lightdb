enable_testing()

include(gtest.cmake)

#find_package(GTest 1.9.1 REQUIRED)

# Include GTest headers
#include_directories(${GTEST_INCLUDE_DIRS})

# Include test header directories
file(GLOB LIGHTDB_TEST_INCLUDE_DIRS "include/*")
include_directories(${LIGHTDB_TEST_INCLUDE_DIRS})

# Include core header directories
file(GLOB LIGHTDB_CORE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/core/*/include/")
include_directories(${LIGHTDB_CORE_INCLUDE_DIRS})

# Get test source files
file(GLOB_RECURSE LIGHTDB_TEST_SOURCES "src/*.cc")

# Build tests
add_executable(lightdb_test EXCLUDE_FROM_ALL ${LIGHTDB_TEST_SOURCES})
target_link_libraries(
    lightdb_test   gtest #${GTEST_BOTH_LIBRARIES}
    lightdb_static ${LIGHTDB_LIB_DEPENDENCIES}
)
add_test(lightdb_test lightdb_test)

# Copy test files to output directory
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(lightdb_test PRIVATE LIGHTDB_BENCHMARK_DATASET_PATH="${CMAKE_SOURCE_DIR}/benchmarks/datasets")

# Build plugins used by tests
include(ExternalProject)
file(GLOB LIGHTDB_PLUGIN_DIRS ${CMAKE_SOURCE_DIR}/plugins/*)
foreach(LIGHTDB_PLUGIN_DIR ${LIGHTDB_PLUGIN_DIRS})
    if (IS_DIRECTORY ${LIGHTDB_PLUGIN_DIR})
        get_filename_component(LIGHTDB_PLUGIN_NAME ${LIGHTDB_PLUGIN_DIR} NAME_WE)
        string(TOLOWER "${LIGHTDB_PLUGIN_DIR}/cmake-build-${CMAKE_BUILD_TYPE}" LIGHTDB_PLUGIN_BINARY_DIR)
        ExternalProject_Add(
                ${LIGHTDB_PLUGIN_NAME}
                PREFIX ${LIGHTDB_PLUGIN_DIR}
                SOURCE_DIR ${LIGHTDB_PLUGIN_DIR}
                BINARY_DIR ${LIGHTDB_PLUGIN_BINARY_DIR}
                INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins
                CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                EXCLUDE_FROM_ALL TRUE
                BUILD_ALWAYS TRUE
                INSTALL_COMMAND ""
        )
        add_dependencies(lightdb_test ${LIGHTDB_PLUGIN_NAME})
    endif(IS_DIRECTORY ${LIGHTDB_PLUGIN_DIR})
endforeach(LIGHTDB_PLUGIN_DIR)

# Copy modules to output directory
ADD_CUSTOM_COMMAND(TARGET lightdb_test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/modules
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/modules/*.ptx ${CMAKE_CURRENT_BINARY_DIR}/modules
        )

# Add custom target 'check'
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V DEPENDS lightdb_test)

# Add custom target for CUDA memcheck
find_program(CUDAMEMCHECK "cuda-memcheck")
if(CUDAMEMCHECK)
    add_custom_target(cuda-memcheck
        COMMAND "${CUDAMEMCHECK}" --error-exitcode 1 --flush-to-disk yes --leak-check full $<TARGET_FILE:lightdb_test>)
endif()

find_program(VALGRIND "valgrind")
if(VALGRIND)
    add_custom_target(valgrind
        COMMAND "${VALGRIND}" --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $<TARGET_FILE:lightdb_test>)
endif()
