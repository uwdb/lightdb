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

TEST_F(OperatorTestFixture, testIdentityEncode) {
    auto video = Decode<EquirectangularGeometry>("resources/red10.h264").apply();

    ASSERT_EQ(video->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, 99, 0, 0}), YUVColor::Null);

    auto result = video >> Encode<YUVColorSpace>("h264");

    //try {
        //visualcloud::SingletonFileEncodedLightField slf = dynamic_cast<visualcloud::SingletonFileEncodedLightField>(*result);
        /*EncodedLightFieldData &v = *result;
        auto v2 = v.bytes();
        auto v3 = v2->size();
        auto e = result->bytes();
        auto e2 = e.size();
        result->bytes()->size();
        auto c = result->bytes()->size();*/
    //} catch(...) {
    //    printf("sigh\n");
    //}
    //ASSERT_GT(result->encodings().size(), 0);

    /*EncodedLightFieldData &vx = *result;
    auto s = result->bytes();
    LOG(INFO) << "a" << vx.bytes()->size();
    LOG(INFO) << "b" << SingletonFileEncodedLightField::create("resources/red10.h264")->bytes()->size();
    auto x = *SingletonFileEncodedLightField::create("resources/red10.h264")->bytes();
    auto y = *result->bytes();
    auto z = x == y;*/

    ASSERT_GT(result->bytes().size(), 0);
    ASSERT_EQ(result->bytes(), SingletonFileEncodedLightField::create("resources/red10.h264")->bytes());
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

    ASSERT_GT(result->bytes().size(), 0);
    ASSERT_EQ(result->bytes(), SingletonFileEncodedLightField::create("resources/red10-green10.h264")->bytes());
}

TEST_F(OperatorTestFixture, test360VerticalTiling) {
    auto name = "result";
    auto theta = rational(102928, 2*32763);
    std::vector bitrates{50, 50, 5000, 50};
    auto i = 0u;

    Decode<EquirectangularGeometry>("resources/test-pattern.h264")
            >> Select(Point3D::Zero)
            >> Partition(Dimension::Time, 1)
            >> Partition(Dimension::Theta, theta)
            >> Transcode([i, bitrates](auto& volume) mutable { return bitrates[i++]; })
            >> Interpolate(Dimension::Time, interpolation::NearestNeighbor)
            >> Discretize(Dimension::Time, rational(1, 60))
            >> Partition(Dimension::Time, 1)
            >> Encode<YUVColorSpace>()
            >> Store(name);

    EXPECT_VIDEO_VALID(name);
    EXPECT_VIDEO_RESOLUTION(name, 952, 1904); //TODO
    EXPECT_EQ(remove(name), 0);
}

TEST_F(OperatorTestFixture, test360HorizontalTiling) {
    auto name = "result";
    auto phi = rational(102928, 4*32763);
    std::vector bitrates{50, 50, 5000, 50};
    auto i = 0u;

    Decode<EquirectangularGeometry>("resources/test-pattern.h264")
            >> Select(Point3D::Zero)
            >> Partition(Dimension::Time, 1)
            >> Partition(Dimension::Phi, phi)
            >> Transcode([i, bitrates](auto& volume) mutable { return bitrates[i++]; })
            >> Interpolate(Dimension::Time, interpolation::NearestNeighbor)
            >> Discretize(Dimension::Time, rational(1, 60))
            >> Partition(Dimension::Time, 1)
            >> Encode<YUVColorSpace>()
            >> Store(name);

    EXPECT_VIDEO_VALID(name);
    EXPECT_VIDEO_RESOLUTION(name, 942, 1920); //TODO
    EXPECT_EQ(remove(name), 0);
}

TEST_F(OperatorTestFixture, test360Tiling) {
    auto name = "result";
    auto theta = rational(102928, 4*32763);
    auto phi = rational(102928, 4*32763);
    std::vector bitrates{50, 50, 50, 50, 50, 1000, 5000, 50, 50, 50, 50, 50};
    auto i = 0u;

    Decode<EquirectangularGeometry>("resources/test-pattern.h264")
            >> Select(Point3D::Zero)
            >> Partition(Dimension::Time, 1)
            >> Partition(Dimension::Phi, phi)
            >> Partition(Dimension::Theta, theta)
            >> Transcode([i, bitrates](auto& volume) mutable { return bitrates[i++]; })
            >> Interpolate(Dimension::Time, interpolation::NearestNeighbor)
            >> Discretize(Dimension::Time, rational(1, 60))
            >> Partition(Dimension::Time, 1)
            >> Encode<YUVColorSpace>()
            >> Store(name);

    EXPECT_VIDEO_VALID(name);
    EXPECT_VIDEO_RESOLUTION(name, 942, 1904); // TODO
    EXPECT_EQ(remove(name), 0);
}

TEST_F(OperatorTestFixture, test360TilingBenchmarkAt1K) {
    auto name = "result";
    auto source = "resources/test-pattern-1K.h264";
    auto theta = rational(102928, 2*32763);
    auto phi = rational(102928, 4*32763);
    std::vector bitrates{50, 50, 50, 50, 50, 1000, 5000, 50, 50, 50, 50, 50};
    auto i = 0u;

    visualcloud::physical::EquirectangularTiledLightField<YUVColorSpace>::hardcode_hack(30, 30, 1920, 3840, 6, 8, 1, "h264", "hevc");
    //visualcloud::physical::EquirectangularTiledLightField<YUVColorSpace>::hardcode_hack(30, 30, 1920, 3840, 4, 4, 50, "h264", "hevc");
    //::google::InitGoogleLogging("visualcloud");

    auto start = std::chrono::steady_clock::now();

    Decode<EquirectangularGeometry>(source)
            >> Select(Point3D::Zero)
            >> Partition(Dimension::Time, 1)
            >> Partition(Dimension::Phi, phi)
            >> Partition(Dimension::Theta, theta)
            >> Transcode([i, bitrates](auto&) mutable { return bitrates[i++]; })
            >> Interpolate(Dimension::Time, interpolation::NearestNeighbor)
            >> Discretize(Dimension::Time, rational(1, 60))
            >> Partition(Dimension::Time, 1)
            >> Encode<YUVColorSpace>()
            >> Store(name);

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    std::cout << elapsed.count() << "ms" << std::endl;

    EXPECT_VIDEO_VALID(name);
    //EXPECT_VIDEO_FRAMES(name, 600);
    //EXPECT_VIDEO_RESOLUTION(name, 2160, 3840);
    //EXPECT_VIDEO_QUALITY(name, source, 30);
    //EXPECT_EQ(remove(name.c_str()), 0);

    //EXPECT_VIDEO_VALID(name);
    //EXPECT_VIDEO_RESOLUTION(name, 942, 1904); // TODO
    //EXPECT_EQ(remove(name), 0);
}

TEST_F(OperatorTestFixture, test360TilingBenchmarkAt2K) {
    auto name = "result";
    auto source = "resources/test-pattern-2K.h264";
    auto theta = rational(102928, 2*32763);
    auto phi = rational(102928, 4*32763);
    std::vector bitrates{50, 50, 50, 50, 50, 1000, 5000, 50, 50, 50, 50, 50};
    auto i = 0u;

    visualcloud::physical::EquirectangularTiledLightField<YUVColorSpace>::hardcode_hack(30, 30, 1920, 3840, 6, 8, 1, "h264", "hevc");
    //visualcloud::physical::EquirectangularTiledLightField<YUVColorSpace>::hardcode_hack(30, 30, 1920, 3840, 4, 4, 50, "h264", "hevc");
    //::google::InitGoogleLogging("visualcloud");

    auto start = std::chrono::steady_clock::now();

    Decode<EquirectangularGeometry>(source)
            >> Select(Point3D::Zero)
            >> Partition(Dimension::Time, 1)
            >> Partition(Dimension::Phi, phi)
            >> Partition(Dimension::Theta, theta)
            >> Transcode([i, bitrates](auto&) mutable { return bitrates[i++]; })
            >> Interpolate(Dimension::Time, interpolation::NearestNeighbor)
            >> Discretize(Dimension::Time, rational(1, 60))
            >> Partition(Dimension::Time, 1)
            >> Encode<YUVColorSpace>()
            >> Store(name);

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    std::cout << elapsed.count() << "ms" << std::endl;

    EXPECT_VIDEO_VALID(name);
    //EXPECT_VIDEO_FRAMES(name, 600);
    //EXPECT_VIDEO_RESOLUTION(name, 2160, 3840);
    //EXPECT_VIDEO_QUALITY(name, source, 30);
    //EXPECT_EQ(remove(name.c_str()), 0);

    //EXPECT_VIDEO_VALID(name);
    //EXPECT_VIDEO_RESOLUTION(name, 942, 1904); // TODO
    //EXPECT_EQ(remove(name), 0);
}

TEST_F(OperatorTestFixture, test360TilingBenchmarkAt4K) {
    auto name = "result";
    auto source = "resources/test-pattern-4K.h264";
    auto theta = rational(102928, 2*32763);
    auto phi = rational(102928, 4*32763);
    std::vector bitrates{50, 50, 50, 50, 50, 1000, 5000, 50, 50, 50, 50, 50};
    auto i = 0u;

    visualcloud::physical::EquirectangularTiledLightField<YUVColorSpace>::hardcode_hack(30, 30, 1920, 3840, 6, 8, 1, "h264", "hevc");
    //visualcloud::physical::EquirectangularTiledLightField<YUVColorSpace>::hardcode_hack(30, 30, 1920, 3840, 4, 4, 50, "h264", "hevc");
    //::google::InitGoogleLogging("visualcloud");

    auto start = std::chrono::steady_clock::now();

    Decode<EquirectangularGeometry>(source)
            >> Select(Point3D::Zero)
            >> Partition(Dimension::Time, 1)
            >> Partition(Dimension::Phi, phi)
            >> Partition(Dimension::Theta, theta)
            >> Transcode([i, bitrates](auto&) mutable { return bitrates[i++]; })
            >> Interpolate(Dimension::Time, interpolation::NearestNeighbor)
            >> Discretize(Dimension::Time, rational(1, 60))
            >> Partition(Dimension::Time, 1)
            >> Encode<YUVColorSpace>()
            >> Store(name);

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    std::cout << elapsed.count() << "ms" << std::endl;

    EXPECT_VIDEO_VALID(name);
    //EXPECT_VIDEO_FRAMES(name, 600);
    //EXPECT_VIDEO_RESOLUTION(name, 2160, 3840);
    //EXPECT_VIDEO_QUALITY(name, source, 30);
    //EXPECT_EQ(remove(name.c_str()), 0);

    //EXPECT_VIDEO_VALID(name);
    //EXPECT_VIDEO_RESOLUTION(name, 942, 1904); // TODO
    //EXPECT_EQ(remove(name), 0);
}

TEST_F(OperatorTestFixture, test360TilingBenchmarkAt8K) {
    auto name = "result";
    auto source = "resources/test-pattern-8K.h264";
    auto theta = rational(102928, 2*32763);
    auto phi = rational(102928, 4*32763);
    std::vector bitrates{50, 50, 50, 50, 50, 1000, 5000, 50, 50, 50, 50, 50};
    auto i = 0u;

    //visualcloud::physical::EquirectangularTiledLightField<YUVColorSpace>::hardcode_hack(30, 30, 1920, 3840, 6, 8, 1, "h264", "hevc");
    //visualcloud::physical::EquirectangularTiledLightField<YUVColorSpace>::hardcode_hack(30, 30, 1920, 3840, 4, 4, 50, "h264", "hevc");
    //::google::InitGoogleLogging("visualcloud");

    auto start = std::chrono::steady_clock::now();

    Decode<EquirectangularGeometry>(source)
            >> Select(Point3D::Zero)
            >> Partition(Dimension::Time, 1)
            >> Partition(Dimension::Phi, phi)
            >> Partition(Dimension::Theta, theta)
            >> Transcode([i, bitrates](auto&) mutable { return bitrates[i++]; })
            >> Interpolate(Dimension::Time, interpolation::NearestNeighbor)
            >> Discretize(Dimension::Time, rational(1, 60))
            >> Partition(Dimension::Time, 1)
            >> Encode<YUVColorSpace>()
            >> Store(name);

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    std::cout << elapsed.count() << "ms" << std::endl;

    EXPECT_VIDEO_VALID(name);
    //EXPECT_VIDEO_FRAMES(name, 600);
    //EXPECT_VIDEO_RESOLUTION(name, 2160, 3840);
    //EXPECT_VIDEO_QUALITY(name, source, 30);
    //EXPECT_EQ(remove(name.c_str()), 0);

    //EXPECT_VIDEO_VALID(name);
    //EXPECT_VIDEO_RESOLUTION(name, 942, 1904); // TODO
    //EXPECT_EQ(remove(name), 0);
}

