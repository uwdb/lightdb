#ifndef LIGHTDB_ASSERTTIME_H
#define LIGHTDB_ASSERTTIME_H

#include "gtest/gtest.h"
#include <chrono>

#define ASSERT_USECS(f, usecs) { \
    auto start = std::chrono::high_resolution_clock::now(); \
    { f; }; \
    std::chrono::duration elapsed{std::chrono::high_resolution_clock::now() - start}; \
    auto ticks = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count(); \
    if(ticks > usecs) { \
        FAIL() << "Performance violation (" << ticks << " > " << usecs << " microseconds) for "#f; \
    } }

#define ASSERT_MSECS(f, msecs) ASSERT_USECS(f, (msecs) * 1000)
#define ASSERT_SECS(f, secs) ASSERT_MSECS(f, (secs) * 1000)

#endif //LIGHTDB_ASSERTTIME_H
