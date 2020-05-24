//#include "Operators.h"
#include "PhysicalOperators.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace lightdb;
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
    const rational pi, pi_div_2, pi_div_4;

    //TODO fix
    double temptodouble(const rational &value) {
        return (double)value.numerator() / value.denominator();
    }

    std::string filename(const char *prefix, const size_t size, const size_t duration) {
        return std::string("resources/") + prefix + '-' + std::to_string(size) +
                "K-" + std::to_string(duration) + "s.h264";
    }

    void testStitchedUnion(std::string dataset, size_t size, size_t frames, size_t height, size_t width) {
        //auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s";
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + 'K';

        LOG(INFO) << "Creating stitchable HEVC input";
/*        Decode(source + ".h264")
                >> Encode("hevc")
                >> Store(source + ".hevc");*/

        auto start = steady_clock::now();

/*        auto left = Decode(source + ".hevc", {0, temptodouble(pi)}).apply();
        auto right = Decode(source + ".hevc", {0, temptodouble(pi)}).apply()
                >> Rotate(temptodouble(pi), 0);

        auto result = (left | right)
                >> Encode()
                >> Store(name);*/

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

/*        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, 2*width);
        EXPECT_EQ(remove(name), 0);*/
        GTEST_SKIP(); //TODO
    }

    void testTranscodedUnion(std::string dataset1, std::string dataset2, size_t size, size_t frames, size_t height, size_t width) {
        //auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s.h264";
        auto source1 = std::string("../../benchmarks/datasets/") + dataset1 + '/' + dataset1 + std::to_string(size) + "K.h264";
        auto source2 = std::string("../../benchmarks/datasets/") + dataset2 + '/' + dataset2 + std::to_string(size) + "K.h264";

        auto start = steady_clock::now();

/*        auto left = Decode(source1, {0, temptodouble(pi)}).apply();
        auto right = Decode(source2, {0, temptodouble(pi)}).apply();

        auto result = (left | right)
                >> Encode()
                >> Store(name);*/

        LOG(INFO) << source1 << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

/*        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);*/
        GTEST_SKIP(); //TODO
    }

    void testSelfUnion(std::string dataset, size_t size, size_t frames, size_t height, size_t width) {
        //auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s.h264";
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";

        auto start = steady_clock::now();

/*        auto left = Decode(source).apply();
        auto right = Decode(source).apply();

        auto result = (left | right)
                >> Encode("h264")
                >> Store(name);*/

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

/*        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);*/
        GTEST_SKIP(); //TODO
    }
};


// Stitchable union tests

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_1K) {
    testStitchedUnion("timelapse", 1, 2700, 512, 960);
}

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_2K) {
    testStitchedUnion("timelapse", 2, 2700, 1024, 1920);
}

TEST_F(UnionBenchmarkTestFixture, testStitchedUnion_4K) {
    testStitchedUnion("timelapse", 4, 2700, 2048, 3840);
}

// Unstitchable union tests
TEST_F(UnionBenchmarkTestFixture, testTranscodedUnion_4K) {
    testTranscodedUnion("timelapse", "timelapse", 4, 2701, 2048, 3840);
}

TEST_F(UnionBenchmarkTestFixture, testSelfUnion_4K) {
    testSelfUnion("timelapse", 4, 2701, 2048, 3840);
}
