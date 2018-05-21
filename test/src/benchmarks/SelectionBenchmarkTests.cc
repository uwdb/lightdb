#include "Operators.h"
#include "PhysicalOperators.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace lightdb;
using namespace std::chrono;

class SelectionBenchmarkTestFixture : public testing::Test {
public:
    SelectionBenchmarkTestFixture()
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

    void testSelect(std::string dataset,
                    size_t size, size_t frames,
                    size_t height, size_t width,
                    ThetaRange theta, PhiRange phi) {
        //auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s.h264";
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";
        auto left = round((theta.start() / ThetaRange::limits().end()) * width),
             top = round((phi.start() / PhiRange::limits().end()) * height),
             expected_width = round(((theta.end() - theta.start()) / ThetaRange::limits().end()) * width),
             expected_height = round(((phi.end() - phi.start()) / PhiRange::limits().end()) * height);
        LOG(INFO) << "Cropping at " << top << 'x' << left << " to " << expected_height << 'x' << expected_width;

        auto start = steady_clock::now();

        Decode(source)
                >> Select(Volume{Point3D::zero(), TemporalRange::limits(), theta, phi})
                >> Encode()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, expected_height, expected_width);
        EXPECT_EQ(remove(name), 0);
    }

    void testTemporalSelect(std::string dataset,
                            size_t size, size_t frames, size_t fps,
                            size_t height, size_t width,
                            TemporalRange range) {
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";
        LOG(INFO) << "Temporal fragment from " << range.start() << " to " << range.end();

        auto start = steady_clock::now();

        Decode(source)
                >> Select({Point3D::zero(), range, ThetaRange::limits(), PhiRange::limits()})
                >> Encode()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames/(range.magnitude() / fps));
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);
    }
};

TEST_F(SelectionBenchmarkTestFixture, testSelect_1K) {
    testSelect("timelapse", 1, 2701, 512, 960, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect("timelapse", 1, 2701, 512, 960, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect("timelapse", 1, 2701, 512, 960, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect("timelapse", 1, 2701, 512, 960, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testSelect_2K) {
    testSelect("timelapse", 2, 2701, 1024, 1920, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect("timelapse", 2, 2701, 1024, 1920, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect("timelapse", 2, 2701, 1024, 1920, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect("timelapse", 2, 2701, 1024, 1920, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testSelect_4K) {
    testSelect("timelapse", 4, 2701, 2048, 3840, {temptodouble(pi_div_2), temptodouble(pi)}, PhiRange::limits());

    testSelect("timelapse", 4, 2701, 2048, 3840, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect("timelapse", 4, 2701, 2048, 3840, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect("timelapse", 4, 2701, 2048, 3840, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect("timelapse", 4, 2701, 2048, 3840, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testTemporalSelect_4K) {
    testTemporalSelect("timelapse", 4, 2701, 30, 2048, 3840, {0, 2});
}
