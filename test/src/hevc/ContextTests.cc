#include "Context.h"
#include <gtest/gtest.h>

class ContextTestFixture : public testing::Test {
};

TEST_F(ContextTestFixture, testConstructor) {
    const std::pair tile_dimensions{2, 3};
    const std::pair video_dimensions{4, 5};
    lightdb::hevc::Context context(tile_dimensions, video_dimensions);

    ASSERT_EQ(context.GetTileDimensions().first,   2);
    ASSERT_EQ(context.GetTileDimensions().second,  3);
    ASSERT_EQ(context.GetVideoDimensions().first,  2*4);
    ASSERT_EQ(context.GetVideoDimensions().second, 3*5);
}

