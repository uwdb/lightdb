#include "LightField.h"
#include <gtest/gtest.h>

using namespace lightdb;

class VolumeTestFixture : public testing::Test {
public:
    VolumeTestFixture()
    { }
};

TEST_F(VolumeTestFixture, testConstructor) {
    Volume{{1, 2}, {3, 4}, {5, 6}, {7, 8}, {0, 1}, {2, 3}};
}
