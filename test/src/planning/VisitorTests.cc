#include "HeuristicOptimizer.h"
#include "Coordinator.h"
#include "Greyscale.h"
#include "Display.h"
#include "HomomorphicOperators.h"
#include "VisitorTemp2.h"
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
    auto small = input.Discretize(Dimension::Theta, rational_times_real({2, 160}, PI));
    auto gray = small.Map(yolo);
    //auto gray = input.Map(lightdb::Greyscale);
    //auto gray = input.Map(yolo);
    auto encoded = gray.Encode(Codec::boxes());

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    print_plan(plan);

    coordinator.save(plan, "dout.hevc");
}
