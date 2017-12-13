#include "Operators.h"
#include "Physical.h"
#include "Functor.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;
using namespace std::chrono;

class ObjectDetectionBenchmarkTestFixture: public testing::Test {
public:
    ObjectDetectionBenchmarkTestFixture()
        : name("result"),
          pi(102928, 32763)
    { }

    const char *name;
    const rational pi;
};


TEST_F(ObjectDetectionBenchmarkTestFixture, testObjectDetection) {
    auto filename = "../../benchmarks/datasets/timelapse/timelapse4K.h264";
    auto f = visualcloud::ObjectDetect();

    auto start = steady_clock::now();

    auto source = Decode<EquirectangularGeometry>(filename).apply();
    auto yolo = source //>> Discretize(Dimension::Theta, rational(2*pi.numerator(), 480*pi.denominator()))
                       //>> Discretize(Dimension::Phi,   rational(pi.numerator(),   240*pi.denominator()))
                       >> Map(f);

    auto result = yolo >> Encode<YUVColorSpace>("h264") >> Store(name);

    //auto result = Union(Union::MergeType::Left).apply(source, yolo)
    //        >> Encode<YUVColorSpace>("h264")
    //        >> Store(name);

    LOG(INFO) << filename << " time:" << ::duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms";

    EXPECT_VIDEO_VALID(name);
    EXPECT_VIDEO_FRAMES(name, 2701);
    EXPECT_VIDEO_RESOLUTION(name, 2048, 3840);
    //EXPECT_EQ(remove(name), 0);
}
