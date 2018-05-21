#include "Operators.h"
#include "PhysicalOperators.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace lightdb;
using namespace std::chrono;

class PartitionBenchmarkTestFixture : public testing::Test {
public:
    PartitionBenchmarkTestFixture()
            : name("result"),
              pi_div_2(102928, 2*32763),
              pi_div_4(102928, 4*32763)
    { }

    const char *name;
    const rational pi_div_2, pi_div_4;

    void partitioningThetaBenchmark(std::string dataset, size_t size, size_t frames, size_t height, size_t width) {
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";
        auto start = steady_clock::now();

        Decode(source)
                >> Partition(Dimension::Theta, pi_div_2)
                >> Encode()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        //EXPECT_VIDEO_FRAMES(name, frames);
        //EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);
    }

    void partitioningPhiBenchmark(std::string dataset, size_t size, size_t frames, size_t height, size_t width) {
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";
        auto start = steady_clock::now();

        Decode(source)
                >> Partition(Dimension::Phi, pi_div_4)
                >> Encode()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        //EXPECT_VIDEO_FRAMES(name, frames);
        //EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);
    }

    void partitioningTemporalBenchmark(std::string dataset, size_t size, size_t frames, size_t height, size_t width) {
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";
        auto start = steady_clock::now();

        Decode(source)
                >> Partition(Dimension::Time, rational(3, 2))
                >> Encode()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        //EXPECT_VIDEO_FRAMES(name, frames);
        //EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);
    }
};

TEST_F(PartitionBenchmarkTestFixture, testPartitionBenchmark_1K) {
    partitioningThetaBenchmark("timelapse", 1, 2700, 512, 1024/4);
    partitioningPhiBenchmark("timelapse", 1, 2700, 512/4, 1024);
}

TEST_F(PartitionBenchmarkTestFixture, testPartitionBenchmark_2K) {
    partitioningThetaBenchmark("timelapse", 2, 2700, 1024, 2048/4);
    partitioningPhiBenchmark("timelapse", 2, 2700, 1024/4, 2048);
}

TEST_F(PartitionBenchmarkTestFixture, testPartitionBenchmark_4K) {
    //partitioningPhiBenchmark("timelapse", 4, 2700, 1920/4, 3840);
    //partitioningBenchmark("timelapse", 4, 2700, 1920, 3840/4);
    partitioningTemporalBenchmark("timelapse", 4, 2700, 1920, 3840);
}
