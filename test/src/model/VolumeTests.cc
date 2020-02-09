#include "LightField.h"
#include <gtest/gtest.h>

using namespace lightdb;

class VolumeTestFixture : public testing::Test {
public:
    VolumeTestFixture() = default;
};

TEST_F(VolumeTestFixture, testConstructor) {
    Volume{{1, 2}, {3, 4}, {5, 6}, {7, 8}, {0, 1}, {2, 3}};
}

TEST_F(VolumeTestFixture, testTemporalIntersection) {
    Volume v = Volume::zero();

    TemporalRange t1{-10, 10};
    TemporalRange t2{  0, 10};
    TemporalRange t3{-10,  0};
    ASSERT_EQ((v | t1).t(), t1);
    ASSERT_EQ((v | t2).t(), t2);
    ASSERT_EQ((v | t3).t(), t3);

    TemporalRange t4{-10, -2};
    TemporalRange t5{  2, 10};
    ASSERT_EQ((v | t4).t(), TemporalRange(-10,  0));
    ASSERT_EQ((v | t5).t(), TemporalRange(  0, 10));
}
