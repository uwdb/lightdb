#include "Operators.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;

class PanoramicTestFixture : public testing::Test {
public:
    PanoramicTestFixture()
    { }
};

TEST_F(PanoramicTestFixture, testPanoramicConstructor) {
    PanoramicVideoLightField("resources/test-pattern.h264");
}

/*
TEST_F(PanoramicTestFixture, testPanoramicValueOutOfVolume) {
    auto field = PanoramicVideoLightField("resources/test-pattern.h264",
                                                                                  {0, 0, 0});

    ASSERT_EQ(field.value({-1,  0,  0,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0, -1,  0,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0, -1,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0, -1,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0,  0, -1,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0,  0,  0, -1}), YUVColor::Null);
    ASSERT_EQ(field.value({ 1,  1,  1,  1,  1,  1}), YUVColor::Null);
    ASSERT_EQ(field.value({-1, -1, -1, -1, -1, -1}), YUVColor::Null);
}

TEST_F(PanoramicTestFixture, testPanoramicValue) {
    auto field = ConstantLightField::create(YUVColor::Green);

    ASSERT_NE(field->value({0, 0, 0, 0, 0, 0}), YUVColor::Null);
    ASSERT_EQ(field->value({0, 0, 0, 0, 0, 0}), YUVColor::Green);
}
*/

TEST_F(PanoramicTestFixture, testConstantEncode) {
    auto color = YUVColor::green();
    auto result = ConstantLightField::create(color, Volume{Point6D::zero()}) >> Encode("YUV");

    ASSERT_EQ(YUVColor(*result->bytes()), color);
}

