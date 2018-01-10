#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;

class OperatorTestFixture : public testing::Test {
public:
    OperatorTestFixture()
    { }
};

TEST_F(OperatorTestFixture, testDecode) {
    auto video = Decode<EquirectangularGeometry>("resources/red10.h264").apply();
    auto *discrete = dynamic_cast<PanoramicVideoLightField<EquirectangularGeometry, YUVColorSpace>*>(&*video);

    ASSERT_NE(discrete, nullptr);

    //TODO fix this after adding rational overloads
    auto fps = (double)discrete->framerate().numerator() / discrete->framerate().denominator();

    ASSERT_EQ(video->value({0, 0, 0, 0,     0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, fps*5, 0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, fps/2, 0, 0}), YUVColor::Null);
    ASSERT_EQ(video->value({0, 0, 0, 99,    0, 0}), YUVColor::Null);
}


TEST_F(OperatorTestFixture, testUnion) {
    auto red = Decode<EquirectangularGeometry>("resources/red10.h264").apply();
    auto green = ConstantLightField<YUVColorSpace>::create(YUVColor::Green);

    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);

    auto combined = red | green;

    ASSERT_EQ(combined->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(combined->value({0, 0, 0, 99, 0, 0}), YUVColor::Green);
}

TEST_F(OperatorTestFixture, testSelect) {
    auto green = ConstantLightField<YUVColorSpace>::create(YUVColor::Green);

    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(green->value({0, 0, 0, 99, 0, 0}), YUVColor::Green);

    auto selected = green >> Select(Point3D::Zero.ToVolume({0, 1}));

    ASSERT_EQ(selected->value({0, 0, 0, 0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(selected->value({0, 0, 0, 2, 0, 0}), YUVColor::Null);
}

TEST_F(OperatorTestFixture, testUnionEncode) {
    auto red = Decode<EquirectangularGeometry>("resources/red10.h264").apply();
    auto green = Decode<EquirectangularGeometry>("resources/green10.h264").apply();

    ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);
    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(green->value({0, 0, 0, 99, 0, 0}), YUVColor::Null);

    auto result = (red | green) >> Encode<YUVColorSpace>();

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10-green10.h264", Volume::VolumeMax)->bytes());
}


TEST_F(OperatorTestFixture, testIdentityEncode) {
    auto video = Decode<EquirectangularGeometry>("resources/red10.h264").apply();

    ASSERT_EQ(video->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, 99, 0, 0}), YUVColor::Null);

    auto result = video >> Encode<YUVColorSpace>("h264");

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10.h264", Volume::VolumeMax)->bytes());
}

TEST_F(OperatorTestFixture, testUnionSelect) {
    auto red = Decode<EquirectangularGeometry>("resources/red10.h264").apply();
    auto green = ConstantLightField<YUVColorSpace>::create(YUVColor::Green);

    ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);
    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);

    auto result = (red | green)
            >> Select(Point3D::Zero.ToVolume({0, 20}))
            >> Encode<YUVColorSpace>();

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10-green10.h264", Volume::VolumeMax)->bytes());
}
