#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;
using namespace std::chrono;

class UnionBenchmarkTestFixture: public testing::Test {
public:
    UnionBenchmarkTestFixture()
        : name("result"),
          pi(102928, 32763),
          pi_div_2(102928, 2*32763),
          pi_div_4(102928, 4*32763)
    { }

    const char *name;
    const rational pi, pi_div_4, pi_div_2;

    //TODO fix
    double temptodouble(const rational &value) {
        return (double)value.numerator() / value.denominator();
    }

    std::string filename(const char *prefix, const size_t size, const size_t duration) {
        return std::string("resources/") + prefix + '-' + std::to_string(size) +
                "K-" + std::to_string(duration) + "s.h264";
    }

    void testStitchedUnion(size_t size, size_t duration, size_t frames, size_t height, size_t width) {
        auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s";

        LOG(INFO) << "Creating stitchable HEVC input";
        Decode<EquirectangularGeometry, YUVColorSpace>(source + ".h264")
                >> Encode<YUVColorSpace>("hevc")
                >> Store(source + ".hevc");

        auto start = steady_clock::now();

        auto left = Decode<EquirectangularGeometry>(source + ".hevc", {0, temptodouble(pi)}).apply();
        auto right = Decode<EquirectangularGeometry>(source + ".hevc", {0, temptodouble(pi)}).apply()
                >> Rotate(temptodouble(pi), 0);

        auto result = (left | right)
                >> Encode<YUVColorSpace>()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, 2*width);
        EXPECT_EQ(remove(name), 0);
    }

    void testTranscodedUnion(size_t size, size_t duration, size_t frames, size_t height, size_t width) {
        auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s.h264";

        auto start = steady_clock::now();

        auto left = Decode<EquirectangularGeometry>(source, {0, temptodouble(pi)}).apply();
        auto right = Decode<EquirectangularGeometry>(source, {0, temptodouble(pi)}).apply()
                >> Rotate(temptodouble(pi), 0);

        auto result = (left | right)
                >> Encode<YUVColorSpace>()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, 2*width);
        EXPECT_EQ(remove(name), 0);
    }
};


// Stitchable union tests

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_1K_20s) {
    testStitchedUnion(1, 20, 600, 512, 1024);
}

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_1K_40s) {
    testStitchedUnion(1, 40, 1200, 512, 1024);
}

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_1K_60s) {
    testStitchedUnion(1, 60, 1800, 512, 1024);
}



TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_2K_20s) {
    testStitchedUnion(2, 20, 600, 1024, 2048);
}

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_2K_40s) {
    testStitchedUnion(2, 40, 1200, 1024, 2048);
}

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_2K_60s) {
    testStitchedUnion(2, 60, 1800, 1024, 2048);
}



TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_4K_20s) {
    testStitchedUnion(4, 20, 600, 1920, 3840);
}

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_4K_40s) {
    testStitchedUnion(4, 40, 1200, 1920, 3840);
}

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_4K_60s) {
    testStitchedUnion(4, 60, 1800, 1920, 3840);
}


// Unstitchable union tests
TEST_F(UnionBenchmarkTestFixture, testTranscodedUnion_1K_20s) {
    testTranscodedUnion(1, 20, 600, 512, 1024);
}

