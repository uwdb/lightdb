# Include GTest headers
include_directories(${GTEST_INCLUDE_DIRS})

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
    lightdb_test   ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES}
    lightdb_static ${LIGHTDB_LIB_DEPENDENCIES}
)
add_test(lightdb_test lightdb_test)

# Copy test files to output directory
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy plugins to output plugin directory
ADD_CUSTOM_COMMAND(TARGET lightdb_test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/plugins
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/plugins/* ${CMAKE_CURRENT_BINARY_DIR}/plugins
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
