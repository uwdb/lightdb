#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;
using namespace std::chrono;

class BenchmarkTestFixture : public testing::Test {
public:
    BenchmarkTestFixture()
            : name("result"),
              bitrates{50,   50,   50, 50,
                       50, 1000, 5000, 50,
                       50,   50,   50, 50,
                       50,   50,   50, 50},
              pi_div_2(102928, 2*32763),
              pi_div_4(102928, 4*32763),
              i(0u)
    { }

    const rational pi_div_4, pi_div_2;
    const std::vector<bitrate> bitrates;
    const char *name;
    size_t i;

    void tilingBenchmark(size_t size, size_t duration, size_t frames, size_t height, size_t width) {
        auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s.h264";
        auto start = steady_clock::now();

        Decode<EquirectangularGeometry>(source)
                >> Select(Point3D::Zero)
                >> Partition(Dimension::Time, 1)
                >> Partition(Dimension::Phi, pi_div_4)
                >> Partition(Dimension::Theta, pi_div_2)
                >> Transcode([this](auto&) mutable { return bitrates[i++]; })
                >> Interpolate(Dimension::Time, interpolation::NearestNeighbor)
                >> Discretize(Dimension::Time, rational(1, 60))
                >> Partition(Dimension::Time, 1)
                >> Encode<YUVColorSpace>()
                >> Store(name);

        LOG(INFO) << source << ':' << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);
    }
};

TEST_F(BenchmarkTestFixture, test360TilingBenchmark_1K_20s) {
    tilingBenchmark(1, 20, 600, 512, 1024);
}

TEST_F(BenchmarkTestFixture, test360TilingBenchmark_1K_40s) {
    tilingBenchmark(1, 40, 1200, 512, 1024);
}

TEST_F(BenchmarkTestFixture, test360TilingBenchmark_1K_60s) {
    tilingBenchmark(1, 60, 1800, 512, 1024);
}


TEST_F(BenchmarkTestFixture, test360TilingBenchmark_2K_20s) {
    tilingBenchmark(2, 20, 600, 1024, 2048);
}

TEST_F(BenchmarkTestFixture, test360TilingBenchmark_2K_40s) {
    tilingBenchmark(2, 40, 1200, 1024, 2048);
}

TEST_F(BenchmarkTestFixture, test360TilingBenchmark_2K_620s) {
    tilingBenchmark(2, 60, 1800, 1024, 2048);
}


TEST_F(BenchmarkTestFixture, test360TilingBenchmark_4K_20s) {
    tilingBenchmark(4, 20, 600, 1920, 3840);
}

TEST_F(BenchmarkTestFixture, test360TilingBenchmark_4K_40s) {
    tilingBenchmark(4, 40, 1200, 1920, 3840);
}

TEST_F(BenchmarkTestFixture, test360TilingBenchmark_42K_620s) {
    tilingBenchmark(4, 60, 1800, 1920, 3840);
}

