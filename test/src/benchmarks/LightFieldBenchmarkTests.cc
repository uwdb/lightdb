#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;
using namespace std::chrono;

class LightFieldBenchmarkTestFixture : public testing::Test {
public:
    LightFieldBenchmarkTestFixture()
            : name("result")
    { }

    const char *name;

    void lightFieldToVideoBenchmark(const char *name) {
        auto source = std::string("resources/test-lightfield.h265");
        auto start = steady_clock::now();

        Decode<EquirectangularGeometry>(true, source)
                >> Select({{0.5, 0.5}, {0.5, 0.5}, {0, 0}, TemporalRange::TemporalMax, AngularRange::ThetaMax, AngularRange::PhiMax})
                >> Encode<YUVColorSpace>("hevc")
                >> Store(name);

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        //EXPECT_VIDEO_FRAMES(name, frames);
        //EXPECT_VIDEO_RESOLUTION(name, height, width);
        //EXPECT_EQ(remove(name), 0);
    }

    void lightFieldToStereoVideoBenchmark(const char *name) {
        auto source = std::string("resources/test-lightfield.h265");
        auto start = steady_clock::now();

        auto field = Decode<EquirectangularGeometry>(true, source);
        auto left =  field >> Select({{0.25, 0.25}, {0.5, 0.5}, {0, 0}});
        auto right = field >> Select({{0.75, 0.75}, {0.5, 0.5}, {0, 0}});

        (left | right) >> Encode<YUVColorSpace>("hevc");

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

        EXPECT_VIDEO_VALID(name);
        //EXPECT_VIDEO_FRAMES(name, frames);
        //EXPECT_VIDEO_RESOLUTION(name, height, width);
        //EXPECT_EQ(remove(name), 0);
    }
};

TEST_F(LightFieldBenchmarkTestFixture, testLightFieldToVideoBenchmark) {
    lightFieldToVideoBenchmark(name);
}

TEST_F(LightFieldBenchmarkTestFixture, testLightFieldToStereoVideoBenchmark) {
    lightFieldToStereoVideoBenchmark(name);
}