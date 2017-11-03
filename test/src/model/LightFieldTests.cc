#include "Operators.h"
#include <fstream>
#include <gtest/gtest.h>

class LightFieldTestFixture : public testing::Test {
public:
    LightFieldTestFixture()
    { }
};

TEST_F(LightFieldTestFixture, testPanoramicConstructor) {
    PanoramicVideoLightField<EquirectangularGeometry, YUVColorSpace>({0, 10}, "resources/test-pattern.h264");
}

TEST_F(LightFieldTestFixture, testPanoramicValueOutOfVolume) {
    auto field = PanoramicVideoLightField<EquirectangularGeometry, YUVColorSpace>({0, 10}, "resources/test-pattern.h264");

    ASSERT_EQ(field.value({-1,  0,  0,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0, -1,  0,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0, -1,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0, -1,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0,  0, -1,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0,  0,  0, -1}), YUVColor::Null);
    ASSERT_EQ(field.value({ 1,  1,  1,  1,  1,  1}), YUVColor::Null);
    ASSERT_EQ(field.value({-1, -1, -1, -1, -1, -1}), YUVColor::Null);
}

TEST_F(LightFieldTestFixture, testPanoramicValue) {
    auto field = ConstantLightField<YUVColorSpace>::create(YUVColor::Green);

    ASSERT_NE(field->value({0, 0, 0, 0, 0, 0}), YUVColor::Null);
    ASSERT_EQ(field->value({0, 0, 0, 0, 0, 0}), YUVColor::Green);
}

