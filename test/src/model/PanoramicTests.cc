#include "Operators.h"
#include <fstream>
#include <gtest/gtest.h>

class PanoramicTestFixture : public testing::Test {
public:
    PanoramicTestFixture()
    { }
};

TEST_F(PanoramicTestFixture, testPanoramicConstructor) {
    PanoramicVideoLightField<EquirectangularGeometry, YUVColorSpace>("resources/test-pattern.h264");
}

TEST_F(PanoramicTestFixture, testPanoramicValueOutOfVolume) {
    auto field = PanoramicVideoLightField<EquirectangularGeometry, YUVColorSpace>("resources/test-pattern.h264",
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
    auto field = ConstantLightField<YUVColorSpace>::create(YUVColor::Green);

    ASSERT_NE(field->value({0, 0, 0, 0, 0, 0}), YUVColor::Null);
    ASSERT_EQ(field->value({0, 0, 0, 0, 0, 0}), YUVColor::Green);
}

