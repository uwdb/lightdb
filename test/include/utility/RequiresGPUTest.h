#ifndef LIGHTDB_REQUIRESGPUTEST_H
#define LIGHTDB_REQUIRESGPUTEST_H

#include "GPUContext.h"
#include "lazy.h"
#include <gtest/gtest.h>

class RequiresGPUTest : public testing::Test {
public:
    explicit RequiresGPUTest(const unsigned int device_id=0u)
        : context([device_id]() { return GPUContext{device_id}; }),
          lock([this]() { return VideoLock(context); })
    {}

    void SetUp() override {
        if(GPUContext::device_count() == 0)
            GTEST_SKIP();
        else {
            // Ensure lazy context is initialized (CUDA assumes a thread context)
            context.value();
        }
    }

protected:
    lightdb::lazy<GPUContext> context;
    lightdb::lazy<VideoLock> lock;
};

#endif //LIGHTDB_REQUIRESGPUTEST_H
