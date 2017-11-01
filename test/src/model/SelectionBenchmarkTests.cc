#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;
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
    const rational pi, pi_div_4, pi_div_2;

    //TODO fix
    double temptodouble(const rational &value) {
        return (double)value.numerator() / value.denominator();
    }

    void testSelect(size_t size, size_t duration, size_t frames, size_t height, size_t width, AngularRange phi, AngularRange theta) {
        auto source = std::string("resources/test-") + std::to_string(size) + "K-" + std::to_string(duration) + "s.h264";
        auto left = std::lround((theta.start / AngularRange::ThetaMax.end) * width),
             top = std::lround((phi.start / AngularRange::PhiMax.end) * height),
             expected_width = std::lround(((theta.end - theta.start) / AngularRange::ThetaMax.end) * width),
             expected_height = std::lround(((phi.end - phi.start) / AngularRange::PhiMax.end) * height);
        LOG(INFO) << "Cropping at " << top << 'x' << left << " to " << expected_height << 'x' << expected_width;

        auto start = steady_clock::now();

        Decode<EquirectangularGeometry>(source)
                >> Select(Point3D::Zero.ToVolume(TemporalRange::TemporalMax, theta, phi))
                >> Encode<YUVColorSpace>()
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        EXPECT_VIDEO_FRAMES(name, frames);
        EXPECT_VIDEO_RESOLUTION(name, expected_height, expected_width);
        EXPECT_EQ(remove(name), 0);
    }

};

TEST_F(SelectionBenchmarkTestFixture, testSelect_1K_20s) {
    testSelect(1, 20, 600, 512, 1024, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(1, 20, 600, 512, 1024, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(1, 20, 600, 512, 1024, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(1, 20, 600, 512, 1024, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testSelect_1K_40s) {
    testSelect(1, 40, 1200, 512, 1024, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(1, 40, 1200, 512, 1024, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(1, 40, 1200, 512, 1024, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(1, 40, 1200, 512, 1024, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testSelect_1K_60s) {
    testSelect(1, 60, 1800, 512, 1024, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(1, 60, 1800, 512, 1024, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(1, 60, 1800, 512, 1024, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(1, 60, 1800, 512, 1024, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}



TEST_F(SelectionBenchmarkTestFixture, testSelect_2K_20s) {
    testSelect(2, 20, 600, 1024, 2048, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(2, 20, 600, 1024, 2048, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(2, 20, 600, 1024, 2048, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(2, 20, 600, 1024, 2048, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testSelect_2K_40s) {
    testSelect(2, 40, 1200, 1024, 2048, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(2, 40, 1200, 1024, 2048, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(2, 40, 1200, 1024, 2048, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(2, 40, 1200, 1024, 2048, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testSelect_2K_60s) {
    testSelect(2, 60, 1800, 1024, 2048, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(2, 60, 1800, 1024, 2048, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(2, 60, 1800, 1024, 2048, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(2, 60, 1800, 1024, 2048, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}



TEST_F(SelectionBenchmarkTestFixture, testSelect_4K_20s) {
    testSelect(4, 20, 600, 1920, 3840, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(4, 20, 600, 1920, 3840, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(4, 20, 600, 1920, 3840, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(4, 20, 600, 1920, 3840, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testSelect_4K_40s) {
    testSelect(4, 40, 1200, 1920, 3840, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(4, 40, 1200, 1920, 3840, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(4, 40, 1200, 1920, 3840, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(4, 40, 1200, 1920, 3840, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(SelectionBenchmarkTestFixture, testSelect_4K_60s) {
    testSelect(4, 60, 1800, 1920, 3840, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect(4, 60, 1800, 1920, 3840, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect(4, 60, 1800, 1920, 3840, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect(4, 60, 1800, 1920, 3840, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

