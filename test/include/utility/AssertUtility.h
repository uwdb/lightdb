#ifndef LIGHTDB_ASSERTUTILITY_H
#define LIGHTDB_ASSERTUTILITY_H

#include <filesystem>

#define REQUIRE_TIMELAPSE_DATASET() \
    REQUIRE_FILE(std::filesystem::absolute(LIGHTDB_BENCHMARK_DATASET_PATH) / "timelapse")

#define REQUIRE_FILE(filename) { \
LOG(ERROR) << filename << std::filesystem::exists(filename); \
    if(!std::filesystem::exists(filename)) \
        GTEST_SKIP(); \
}

#define ASSERT_TYPE(instance, type) \
  ASSERT_NE(dynamic_cast<const type*>(&(instance)), nullptr)

#define EXPECT_TYPE(instance, type) \
  EXPECT_NE(dynamic_cast<type*>(&(instance)), nullptr)

#endif //LIGHTDB_ASSERTUTILITY_H
