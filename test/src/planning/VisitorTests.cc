#include "AssertVideo.h"
#include "HeuristicOptimizer.h"
#include "Greyscale.h"
#include "Display.h"
#include "TestResources.h"
#include "extension.h"
#include "AssertUtility.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class VisitorTestFixture : public testing::Test {
public:
    VisitorTestFixture()
            : catalog("resources") {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

protected:
    Catalog catalog;
};

TEST_F(VisitorTestFixture, testBaz) {
    auto name = "red10";
    auto input = Scan(name);
    auto temporal = input.Select(SpatiotemporalDimension::Time, TemporalRange{2, 5});
    auto encoded = temporal.Encode();

    Coordinator().execute(encoded);
}

TEST_F(VisitorTestFixture, testFoo) {
/*    auto left = Scan("red10");
    auto right = Scan("red10");
    auto unioned = left.Union(right);
    auto encoded = unioned.Encode();
    */
    //auto foo = dlopen("/home/bhaynes/projects/yolo/cmake-build-debug/libyolo.so", RTLD_LAZY | RTLD_GLOBAL);
    //printf( "Could not open file : %s\n", dlerror() );
    auto yolo = lightdb::extensibility::Load("yolo"); //, "/home/bhaynes/projects/yolo/cmake-build-debug");

    auto name = "red10"; //std::getenv("TLFNAME");
    auto input = Scan(name);
    auto continuous = input.Interpolate(Dimension::Theta, interpolation::Linear());
    auto smalltheta = continuous.Discretize(Dimension::Theta, rational_times_real({2, 416}, PI));
    auto small = smalltheta.Discretize(Dimension::Phi, rational_times_real({1, 416}, PI));
//    auto gray = small.Map(yolo);
    auto gray = small.Map(lightdb::Greyscale);
    //auto gray = input.Map(yolo);
    //auto encoded = gray.Encode(Codec::boxes());
    auto encoded = gray.Encode(); //Codec::boxes());

    Coordinator().execute(encoded);
}

TEST_F(VisitorTestFixture, testBar) {
    REQUIRE_GPU();

    auto name = "red10";
    auto input = Scan(name);
    auto input2 = Scan(name);
    auto stored = input.Store("postred10_2");
    auto stored2 = input.Store("postred10_3");


    Coordinator().execute({stored, stored2});
}

TEST_F(VisitorTestFixture, testScanStore) {
    REQUIRE_GPU();

    auto name = "red10";
    auto input = Scan(name);
    auto stored = input.Store("postred10");

    Coordinator().execute(stored);
}

TEST_F(VisitorTestFixture, testScanSave) {
    REQUIRE_GPU();

    auto name = "red10";
    auto input = Scan(name);
    auto stored = input.Encode(Codec::hevc()).Save("dout.mp4");

    Coordinator().execute(stored);
}

TEST_F(VisitorTestFixture, testInterpolateDiscretizeMap) {
    REQUIRE_GPU();

    auto yolo = lightdb::extensibility::Load("yolo");

    auto name = "red10";
    auto input = Scan(name);
    auto continuous_t = input.Interpolate(Dimension::Theta, interpolation::Linear());
    auto continuous = continuous_t.Interpolate(Dimension::Phi, interpolation::Linear());
    auto discrete_t = continuous.Discretize(Dimension::Theta, rational_times_real({2, 416}, PI));
    auto downsampled = discrete_t.Discretize(Dimension::Phi, rational_times_real({1, 416}, PI));
    auto boxes = downsampled.Map(yolo);
    auto encoded = boxes.Encode(Codec::boxes());

    //Coordinator().execute(encoded);
    GTEST_SKIP();
}

TEST_F(VisitorTestFixture, testPartitionEncode) {
    REQUIRE_GPU();

    auto name = "red10";
    auto input = Scan(name);
    auto partitioned = input.Partition(Dimension::Theta, rational_times_real({2, 4}, PI));
    auto encoded = partitioned.Encode(Codec::hevc());

    Coordinator().execute(encoded);
}

TEST_F(VisitorTestFixture, testPartitionPartitionEncode) {
    REQUIRE_GPU();

    auto name = "red10";
    auto input = Scan(name);
    auto partitioned1 = input.Partition(Dimension::Theta, rational_times_real({2, 4}, PI));
    auto partitioned2 = partitioned1.Partition(Dimension::Theta, rational_times_real({2, 4}, PI));
    auto encoded = partitioned2.Encode(Codec::hevc());

    Coordinator().execute(encoded);
}

TEST_F(VisitorTestFixture, testPartitionSubqueryUnion) {
    REQUIRE_GPU();

    auto name = "red10";
    auto input = Scan(name);
    auto partitioned_t = input.Partition(Dimension::Theta, rational_times_real({2, 4}, PI));
    auto partitioned = partitioned_t.Partition(Dimension::Phi, rational_times_real({1, 4}, PI));
    //auto transcoded = partitioned_t.Subquery([](auto l) { return l.Encode(Codec::hevc()); });
    auto transcoded = partitioned.Subquery([](auto l) { return l.Encode(Codec::hevc()); });
    auto encoded = transcoded.Encode(Codec::hevc());

    Coordinator().execute(encoded);
}

TEST_F(VisitorTestFixture, testScanInterpolateDiscretize) {
    REQUIRE_GPU();

    auto outputResolution = 416;

    auto input = Scan(Resources.red10.name);
    auto continuous = input.Interpolate(Dimension::Theta, interpolation::Linear());
    auto smallTheta = continuous.Discretize(Dimension::Theta, rational_times_real({2, outputResolution}, PI));
    auto small = smallTheta.Discretize(Dimension::Phi, rational_times_real({1, outputResolution}, PI));
    auto saved = small.Save(Resources.out.hevc);


    Coordinator().execute(saved);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, outputResolution, outputResolution);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}
