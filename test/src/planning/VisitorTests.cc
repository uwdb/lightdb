#include "HeuristicOptimizer.h"
#include "Greyscale.h"
#include "Display.h"
#include "extension.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class VisitorTestFixture : public testing::Test {
public:
    VisitorTestFixture()
            : catalog("resources")
    { Catalog::instance(catalog); }

protected:
    Catalog catalog;
};

TEST_F(VisitorTestFixture, testBaz) {
    auto name = "red10";
    auto input = Scan(name);
    auto temporal = input.Select(SpatiotemporalDimension::Time, TemporalRange{2, 5});
    auto encoded = temporal.Encode();

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    //print_plan(plan);

    coordinator.save(plan, "out.hevc");
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

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    //print_plan(plan);

    coordinator.save(plan, "dout.hevc");
}

TEST_F(VisitorTestFixture, testBar) {
    auto name = "red10";
    auto input = Scan(name);
    auto input2 = Scan(name);
    auto stored = input.Store("postred10_2");
    auto stored2 = input.Store("postred10_3");

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize({stored, stored2});

    //print_plan(plan);

    coordinator.save(plan, std::vector<std::string>{"dout1.hevc", "dout2.hevc"});
}

TEST_F(VisitorTestFixture, testScanStore) {
    auto name = "red10";
    auto input = Scan(name);
    auto stored = input.Store("postred10");

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(stored);

    coordinator.save(plan, "dout1.hevc");
}

TEST_F(VisitorTestFixture, testInterpolateDiscretizeMap) {
    auto yolo = lightdb::extensibility::Load("yolo");

    auto name = "red10";
    auto input = Scan(name);
    auto continuous_t = input.Interpolate(Dimension::Theta, interpolation::Linear());
    auto continuous = continuous_t.Interpolate(Dimension::Phi, interpolation::Linear());
    auto discrete_t = continuous.Discretize(Dimension::Theta, rational_times_real({2, 416}, PI));
    auto downsampled = discrete_t.Discretize(Dimension::Phi, rational_times_real({1, 416}, PI));
    auto boxes = downsampled.Map(yolo);
    auto encoded = boxes.Encode(Codec::boxes());

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    //print_plan(plan);

    //coordinator.save(plan, "dout.hevc");
    FAIL();
}

TEST_F(VisitorTestFixture, testPartitionEncode) {
    auto name = "red10";
    auto input = Scan(name);
    auto partitioned = input.Partition(Dimension::Theta, rational_times_real({2, 4}, PI));
    auto encoded = partitioned.Encode(Codec::hevc());

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    //print_plan(plan);

    coordinator.save(plan, "out.hevc");
}

TEST_F(VisitorTestFixture, testPartitionPartitionEncode) {
    auto name = "red10";
    auto input = Scan(name);
    auto partitioned1 = input.Partition(Dimension::Theta, rational_times_real({2, 4}, PI));
    auto partitioned2 = partitioned1.Partition(Dimension::Theta, rational_times_real({2, 4}, PI));
    auto encoded = partitioned2.Encode(Codec::hevc());

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    //print_plan(plan);

    coordinator.save(plan, "out.hevc");
}

TEST_F(VisitorTestFixture, testPartitionSubqueryUnion) {
    auto name = "red10";
    auto input = Scan(name);
    auto partitioned_t = input.Partition(Dimension::Theta, rational_times_real({2, 4}, PI));
    auto partitioned = partitioned_t.Partition(Dimension::Phi, rational_times_real({1, 4}, PI));
    //auto transcoded = partitioned_t.Subquery([](auto l) { return l.Encode(Codec::hevc()); });
    auto transcoded = partitioned.Subquery([](auto l) { return l.Encode(Codec::hevc()); });
    auto encoded = transcoded.Encode(Codec::hevc());

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    //print_plan(plan);

    //coordinator.save(plan, "out.hevc");
    FAIL();
}