//#include "Operators.h"
#include "PhysicalOperators.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace lightdb;
using namespace std::chrono;
/*
class PanoramicTilingBenchmarkTestFixture : public testing::Test {
public:
    PanoramicTilingBenchmarkTestFixture()
            : name("result"),
              bitrates{50,   50,   50, 50,
                       50, 1000, 5000, 50,
                       50,   50,   50, 50,
                       50,   50,   50, 50},
              pi(102928, 32763),
              pi_div_2(102928, 2*32763),
              pi_div_4(102928, 4*32763),
              pi_div_8(102928, 8*32763),
              i(0u)
    { }

    const char *name;
    const std::vector<bitrate> bitrates;
    const rational pi, pi_div_2, pi_div_4, pi_div_8;
    size_t i = 0u;

    void tilingBenchmark(std::string dataset, size_t size, size_t frames, size_t height, size_t width, size_t rows, size_t cols) {
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";
        auto start = steady_clock::now();
        rational theta, phi;

        if(rows == 2 && cols == 2) {
            theta = pi;
            phi = pi_div_2;
        } else if(rows == 4 && cols == 4) {
            theta = pi_div_2;
            phi = pi_div_4;
        } else if(rows == 8 && cols == 8) {
            theta = pi_div_4;
            phi = pi_div_8;
        } else
            throw new std::runtime_error("Row/col combination not hardcoded");

        Decode(source)
                >> Select({Point3D::zero(), TemporalRange::limits(), ThetaRange::limits(), PhiRange::limits()})
                >> Partition(Dimension::Time, 1)
                >> Partition(Dimension::Phi, phi)
                >> Partition(Dimension::Theta, theta)
                >> Transcode([this](auto&) mutable { return bitrates[i++]; })
                >> Interpolate(Dimension::Time, interpolation::Linear())
                >> Discretize(Dimension::Time, rational(1, 60))
                >> Partition(Dimension::Time, 1)
                >> Encode()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        //EXPECT_EQ(remove(name), 0);
    }
};

TEST_F(PanoramicTilingBenchmarkTestFixture, test360TilingBenchmark_timelapse_1K) {
    tilingBenchmark("timelapse", 1, 2700, 512, 944, 4, 4);
}

TEST_F(PanoramicTilingBenchmarkTestFixture, test360TilingBenchmark_timelapse_2K) {
    tilingBenchmark("timelapse", 2, 2700, 1024, 1920, 4, 4);
}

TEST_F(PanoramicTilingBenchmarkTestFixture, test360TilingBenchmark_timelapse_4K) {
    //tilingBenchmark("timelapse", 4, 2700, 2048, 3840, 2, 2);
    //tilingBenchmark("timelapse", 4, 2700, 2048, 3840, 4, 4);
    tilingBenchmark("timelapse", 4, 2700, 2048, 3840, 8, 8);
}

TEST_F(PanoramicTilingBenchmarkTestFixture, test360TilingBenchmark_venice_4K) {
    tilingBenchmark("venice", 4, 2700, 2048, 3840, 4, 4);
}

TEST_F(PanoramicTilingBenchmarkTestFixture, test360TilingBenchmark_coaster_4K) {
    tilingBenchmark("coaster", 4, 2700, 2048, 3840, 4, 4);
}
*/