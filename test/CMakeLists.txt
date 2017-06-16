# Include GTest headers
include_directories(${GTEST_INCLUDE_DIRS})

# Include test header directories
file(GLOB VISUALCLOUD_TEST_INCLUDE_DIRS "include/*")
include_directories(${VISUALCLOUD_TEST_INCLUDE_DIRS})

# Include core header directories
file(GLOB VISUALCLOUD_CORE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/core/*/include/")
include_directories(${VISUALCLOUD_CORE_INCLUDE_DIRS})

# Get test source files
file(GLOB_RECURSE VISUALCLOUD_TEST_SOURCES "src/*.cc")

# Build tests
add_executable(visualcloud_test EXCLUDE_FROM_ALL ${VISUALCLOUD_TEST_SOURCES})
target_link_libraries(
    visualcloud_test   ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} 
    visualcloud_static ${VISUALCLOUD_LIB_DEPENDENCIES}
)
add_test(visualcloud_test visualcloud_test)

# Add custom target 'check'
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V DEPENDS visualcloud_test)

# Add custom target for CUDA memcheck
find_program(CUDAMEMCHECK "cuda-memcheck")
if(CUDAMEMCHECK)
    add_custom_target(cuda-memcheck
        COMMAND "${CUDAMEMCHECK}" --error-exitcode 1 --leak-check full $<TARGET_FILE:visualcloud_test>)
endif()

find_program(VALGRIND "valgrind")
if(VALGRIND)
    add_custom_target(valgrind
        COMMAND "${VALGRIND}" --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $<TARGET_FILE:visualcloud_test>)
endif()
