#ifndef LIGHTDB_ASSERTUTILITY_H
#define LIGHTDB_ASSERTUTILITY_H

#include "TestResources.h"
#include <filesystem>

#define REQUIRE_TIMELAPSE_DATASET() { \
    REQUIRE_FILE(Resources.datasets.timelapse.timelapse1k); \
    REQUIRE_FILE(Resources.datasets.timelapse.timelapse2k); \
    REQUIRE_FILE(Resources.datasets.timelapse.timelapse4k); \
}

#define REQUIRE_FILE(filename) { \
    if(!std::filesystem::exists(filename)) \
        GTEST_SKIP(); \
}

#define ASSERT_TYPE(instance, type) \
  ASSERT_NE(dynamic_cast<const type*>(&(instance)), nullptr)

#define EXPECT_TYPE(instance, type) \
  EXPECT_NE(dynamic_cast<type*>(&(instance)), nullptr)

#endif //LIGHTDB_ASSERTUTILITY_H
