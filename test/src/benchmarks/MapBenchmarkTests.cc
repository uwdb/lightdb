#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;
using namespace std::chrono;

class MapBenchmarkTestFixture: public testing::Test {
public:
    MapBenchmarkTestFixture()
        : name("result")
    { }

    const char *name;

    void testMap(size_t size, size_t duration, size_t frames, size_t height, size_t width) {
        auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s.h264";

        auto start = steady_clock::now();

        Decode<EquirectangularGeometry>(source)
                >> Map(visualcloud::Greyscale())
                >> Encode<YUVColorSpace>()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        //EXPECT_EQ(remove(name), 0);
    }

};

TEST_F(MapBenchmarkTestFixture, testMap_1K_20s) {
    testMap(1, 20, 600, 512, 1024);
}

TEST_F(MapBenchmarkTestFixture, testMap_1K_40s) {
    testMap(1, 40, 1200, 512, 1024);
}

TEST_F(MapBenchmarkTestFixture, testMap_1K_60s) {
    testMap(1, 60, 1800, 512, 1024);
}


TEST_F(MapBenchmarkTestFixture, testMap_2K_20s) {
    testMap(2, 20, 600, 1024, 2048);
}

TEST_F(MapBenchmarkTestFixture, testMap_2K_40s) {
    testMap(2, 40, 1200, 1024, 2048);
}

TEST_F(MapBenchmarkTestFixture, testMap_2K_60s) {
    testMap(2, 60, 1800, 1024, 2048);
}



TEST_F(MapBenchmarkTestFixture, testMap_4K_20s) {
    testMap(4, 20, 600, 1920, 3840);
}

TEST_F(MapBenchmarkTestFixture, testMap_4K_40s) {
    testMap(4, 40, 1200, 1920, 3840);
}

TEST_F(MapBenchmarkTestFixture, testMap_4K_60s) {
    testMap(4, 60, 1800, 1920, 3840);
}

