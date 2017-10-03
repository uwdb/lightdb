#include "Operators.h"
#include <fstream>
#include <gtest/gtest.h>

class LightFieldTestFixture : public testing::Test {
public:
    LightFieldTestFixture()
    { }
};

TEST_F(LightFieldTestFixture, testPanoramicConstructor) {
    PanoramicLightField<EquirectangularGeometry, YUVColorSpace>({0, 10});
}

TEST_F(LightFieldTestFixture, testPanoramicValueOutOfVolume) {
    auto field = PanoramicLightField<EquirectangularGeometry, YUVColorSpace>({0, 10});

    ASSERT_EQ(field.value(-1,  0,  0,  0,  0,  0), YUVColor::Null);
    ASSERT_EQ(field.value( 0, -1,  0,  0,  0,  0), YUVColor::Null);
    ASSERT_EQ(field.value( 0,  0, -1,  0,  0,  0), YUVColor::Null);
    ASSERT_EQ(field.value( 0,  0,  0, -1,  0,  0), YUVColor::Null);
    ASSERT_EQ(field.value( 0,  0,  0,  0, -1,  0), YUVColor::Null);
    ASSERT_EQ(field.value( 0,  0,  0,  0,  0, -1), YUVColor::Null);
    ASSERT_EQ(field.value( 1,  1,  1,  1,  1,  1), YUVColor::Null);
    ASSERT_EQ(field.value(-1, -1, -1, -1, -1, -1), YUVColor::Null);
}

TEST_F(LightFieldTestFixture, testPanoramicValue) {
    auto field = ConstantLightField<YUVColorSpace>(YUVColor::Green);

    ASSERT_NE(field.value(0, 0, 0, 0, 0, 0), YUVColor::Null);
    ASSERT_EQ(field.value(0, 0, 0, 0, 0, 0), YUVColor::Green);
}

TEST_F(LightFieldTestFixture, testXXX) {
    //auto field = PanoramicLightField<EquirectangularGeometry, YUVColorSpace>({0, 10});

    auto threesixty = Decode<YUVColorSpace>().apply(std::ifstream{"video.mp4"});
    auto green = ConstantLightField<YUVColorSpace>(YUVColor::Green);

    auto combined = Union().apply(threesixty, green);

    auto encoded = Encode<YUVColorSpace>().apply(combined);

    ASSERT_NE(encoded, nullptr);
}
