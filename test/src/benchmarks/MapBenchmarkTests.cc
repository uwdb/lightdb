#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace lightdb;
using namespace std::chrono;

class MapBenchmarkTestFixture: public testing::Test {
public:
    MapBenchmarkTestFixture()
        : name("result")
    { }

    const char *name;

    void testMapGrayscale(std::string dataset, size_t size, size_t frames, size_t height, size_t width) {
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";

        auto start = steady_clock::now();

        Decode<EquirectangularGeometry>(source)
                >> Map(lightdb::Greyscale())
                >> Encode<YUVColorSpace>()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);
    }

    void testMapGaussianBlur(std::string dataset, size_t size, size_t frames, size_t height, size_t width) {
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";

        auto start = steady_clock::now();

        Decode<EquirectangularGeometry>(source)
                >> Map(lightdb::GaussianBlur())
                >> Encode<YUVColorSpace>()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);
    }
};

TEST_F(MapBenchmarkTestFixture, testMap_1K) {
    testMapGrayscale("timelapse", 1, 2701, 512, 960);
}

TEST_F(MapBenchmarkTestFixture, testMap_2K) {
    testMapGrayscale("timelapse", 2, 2701, 1024, 1920);
}

TEST_F(MapBenchmarkTestFixture, testMap_4K) {
    testMapGrayscale("timelapse", 4, 2701, 2048, 3840);
}

TEST_F(MapBenchmarkTestFixture, testMapBlur_4K) {
    testMapGaussianBlur("timelapse", 4, 2701, 2048, 3840);
}
