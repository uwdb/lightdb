#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;
using namespace std::chrono;

class DegenerateSelectionBenchmarkTestFixture : public testing::Test {
public:
    DegenerateSelectionBenchmarkTestFixture()
        : name("result")
    { }

    const char *name;

    //TODO fix
    double temptodouble(const rational &value) {
        return (double)value.numerator() / value.denominator();
    }

    void testDegenerateSelect(std::string dataset,
                    size_t size, size_t frames,
                    size_t height, size_t width) {
        auto source = std::string("../../benchmarks/datasets/") + dataset + '/' + dataset + std::to_string(size) + "K.h264";

        auto start = steady_clock::now();

        Decode<EquirectangularGeometry>(source)
                >> Select(Point3D::Zero.ToVolume({0, 20}, AngularRange::ThetaMax, AngularRange::PhiMax))
                >> Encode<YUVColorSpace>("h264")
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, height, width);
        EXPECT_EQ(remove(name), 0);
    }

};

TEST_F(DegenerateSelectionBenchmarkTestFixture, testSelect_1K) {
    testDegenerateSelect("timelapse", 1, 2701, 512, 960);
}

TEST_F(DegenerateSelectionBenchmarkTestFixture, testSelect_2K) {
    testDegenerateSelect("timelapse", 2, 2701, 1024, 1920);
}

TEST_F(DegenerateSelectionBenchmarkTestFixture, testSelect_4K) {
    testDegenerateSelect("timelapse", 4, 2701, 2048, 3840);
}
/*
TEST_F(DegenerateSelectionBenchmarkTestFixture, testSelect_2K) {
    testSelect("timelapse", 2, 2730, 1024, 1920, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect("timelapse", 2, 2730, 1024, 1920, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect("timelapse", 2, 2730, 1024, 1920, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect("timelapse", 2, 2730, 1024, 1920, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(DegenerateSelectionBenchmarkTestFixture, testSelect_4K) {
    testSelect("timelapse", 4, 2730, 2048, 3840, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect("timelapse", 4, 2730, 2048, 3840, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect("timelapse", 4, 2730, 2048, 3840, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect("timelapse", 4, 2730, 2048, 3840, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}
*/