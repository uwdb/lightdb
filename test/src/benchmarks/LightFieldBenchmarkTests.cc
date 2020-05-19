//#include "Operators.h"
#include "PhysicalOperators.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace lightdb;
using namespace std::chrono;

class LightFieldBenchmarkTestFixture : public testing::Test {
public:
    LightFieldBenchmarkTestFixture()
            : name("result")
    { }

    const char *name;

    void lightFieldToVideoBenchmark(const char *name) {
        auto source = std::string("../../benchmark/datasets/cats/cats.h264");
        auto start = steady_clock::now();

/*        Decode(true, source)
                >> Select({{0.5, 0.5}, {0.5, 0.5}, {0, 0}, TemporalRange::limits(), ThetaRange::limits(), PhiRange::limits()})
                >> Encode("hevc")
                >> Store(name);*/

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

//        EXPECT_VIDEO_VALID(name);
        //EXPECT_VIDEO_FRAMES(name, frames);
        //EXPECT_VIDEO_RESOLUTION(name, height, width);
        //EXPECT_EQ(remove(name), 0);
        GTEST_SKIP();
    }

    void lightFieldToStereoVideoBenchmark(const char *name) {
        auto source = std::string("resources/test-lightfield.h265");
        auto start = steady_clock::now();

/*        auto field = Decode(true, source);
        auto left =  field >> Select({{0.25, 0.25}, {0.5, 0.5}, {0, 0}});
        auto right = field >> Select({{0.75, 0.75}, {0.5, 0.5}, {0, 0}});

        (left | right) >> Encode("hevc");*/

        LOG(INFO) << source << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

//        EXPECT_VIDEO_VALID(name);
        //EXPECT_VIDEO_FRAMES(name, frames);
        //EXPECT_VIDEO_RESOLUTION(name, height, width);
        //EXPECT_EQ(remove(name), 0);
        GTEST_SKIP();
    }
};

TEST_F(LightFieldBenchmarkTestFixture, testLightFieldToVideoBenchmark) {
    lightFieldToVideoBenchmark(name);
}

TEST_F(LightFieldBenchmarkTestFixture, testLightFieldToStereoVideoBenchmark) {
    lightFieldToStereoVideoBenchmark(name);
}
