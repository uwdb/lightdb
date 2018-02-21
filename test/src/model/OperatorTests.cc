#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace lightdb;
using namespace lightdb::logical;

class OperatorTestFixture : public testing::Test {
public:
    OperatorTestFixture()
    { }
};

TEST_F(OperatorTestFixture, testDecode) {
    auto video = Decode("resources/red10.h264").apply();
    auto *discrete = dynamic_cast<PanoramicVideoLightField*>(&*video);

    ASSERT_NE(discrete, nullptr);

    //TODO fix this after adding rational overloads
    //auto fps = (long double)discrete->framerate(); //.numerator() / discrete->framerate().denominator();

    //TODO
    /*ASSERT_EQ(video->value({0, 0, 0, 0,     0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, fps*5, 0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, fps/2, 0, 0}), YUVColor::Null);
    ASSERT_EQ(video->value({0, 0, 0, 99,    0, 0}), YUVColor::Null);*/
}

TEST_F(OperatorTestFixture, testUnion) {
    auto red = Decode("resources/red10.h264").apply();
    auto green = ConstantLightField::create(YUVColor::green());

    //TODO
    /*ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);*/

    auto combined = red | green;

    //TODO
    /*ASSERT_EQ(combined->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(combined->value({0, 0, 0, 99, 0, 0}), YUVColor::Green);*/
}

TEST_F(OperatorTestFixture, testSelect) {
    auto green = ConstantLightField::create(YUVColor::green());

    //TODO
    /*ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(green->value({0, 0, 0, 99, 0, 0}), YUVColor::Green);*/

    auto selected = green >> Select(Volume{Point3D::zero(), {0, 1}, ThetaRange::limits(), PhiRange::limits()});

    //TODO
    /*ASSERT_EQ(selected->value({0, 0, 0, 0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(selected->value({0, 0, 0, 2, 0, 0}), YUVColor::Null);*/
}

TEST_F(OperatorTestFixture, testUnionEncode) {
    auto red = Decode("resources/red10.h264").apply();
    auto green = Decode("resources/green10.h264").apply();

    //TODO
    /*ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);
    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(green->value({0, 0, 0, 99, 0, 0}), YUVColor::Null);*/

    auto result = (red | green) >> Encode();

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10-green10.h264", Volume::limits())->bytes());
}

TEST_F(OperatorTestFixture, testIdentityEncode) {
    auto video = Decode("resources/red10.h264").apply();

    //TODO
    /*ASSERT_EQ(video->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, 99, 0, 0}), YUVColor::Null);*/

    auto result = video >> Encode("h264");

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10.h264", Volume::limits())->bytes());
}

TEST_F(OperatorTestFixture, testUnionSelect) {
    auto red = Decode("resources/red10.h264").apply();
    auto green = ConstantLightField::create(YUVColor::green());

    //TODO
    /*ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);
    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);*/

    auto result = (red | green)
            >> Select({Point3D::zero(), {0, 20}, ThetaRange::limits(), PhiRange::limits()})
            >> Encode();

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10-green10.h264", Volume::limits())->bytes());
}
