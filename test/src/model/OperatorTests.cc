#include "Operators.h"
#include <gtest/gtest.h>

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

TEST_F(OperatorTestFixture, testIdentityEncode) {
    auto video = Decode<EquirectangularGeometry>("resources/red10.h264").apply();

    ASSERT_EQ(video->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, 99, 0, 0}), YUVColor::Null);

    auto encoded = video >> Encode<YUVColorSpace>();

    ASSERT_GT(encoded.bytes().size(), 0);
    ASSERT_EQ(encoded.bytes(), EncodedLightField("resources/red10.h264").bytes());
}

TEST_F(OperatorTestFixture, testIdentityUnionEncode) {
    auto red = Decode<EquirectangularGeometry>("resources/red10.h264").apply();
    auto green = ConstantLightField<YUVColorSpace>::create(YUVColor::Green);

    ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);
    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);

    auto result = (red | green)
            >> Select(Point3D::Zero.ToVolume({0, 20}))
            >> Encode<YUVColorSpace>();

    ASSERT_GT(result.bytes().size(), 0);
    ASSERT_EQ(result.bytes(), EncodedLightField("resources/red10-green10.h264").bytes());
}

TEST_F(OperatorTestFixture, test360Tiling) {
    /*
    auto result =
        Decode<EquirectangularGeometry>("resources/red10.h264")
          >> Select(Point3D{0, 0, 0})
          >> Partition(Dimension::Time, 1)
          >> Partition(Dimension::Theta, 90)
          >> Transcode(std::vector<unsigned int>{50u, 50u, 5000u, 50u})
          >> Interpolate(Interpolate::NearestNeighbor)
          >> Discretize(Dimension::Time, rational(1, 60))
          >> Partition(Dimension::Time, rational(1, 60))
          >> Encode<YUVColorSpace>();
*/
/*
    auto input = Decode<YUVColorSpace, EquirectangularGeometry>().apply(std::ifstream{"resources/red10.h264"});

    auto threesixty = Select().apply(input, {{0, 0}, {0, 0}, {0, 0}, {0, 20}, AngularRange::ThetaMax, AngularRange::PhiMax});
    auto timePartitioned = Partition().applyTime(threesixty, 1);
    auto slices = Partition().applyTheta(timePartitioned, 90);
    auto transcoded = Transcode().apply(slices, {50, 50, 5000, 50});
    auto interpolated = Interpolate().apply(transcoded, Interpolate::NearestNeighbor);
    auto sample60fps = Select().applyFPS(60);
    auto dashable = Partition().applyTime(sample60fps, 1);
    auto encoded = Encode<YUVColorSpace>().apply(dashable);
*/
}

TEST_F(OperatorTestFixture, temp) {
    auto query =
        Decode<EquirectangularGeometry>("resources/red10.h264")
            >> Select(Point3D::Zero)
            >> Partition(Dimension::Time, 1)
            >> Partition(Dimension::Theta, 90)
            //>> Transcode([](auto& volume) { return 50; })
            >> Interpolate([](auto&, auto&) { return YUVColor::Green; })
            >> Discretize(Dimension::Time, rational(1, 60))
            >> Partition(Dimension::Time, rational(1, 60))
            ;

    auto result = query >> Encode<YUVColorSpace>();

    auto &cs = query->colorSpace();
    printf("%ld\n", &cs);
}
