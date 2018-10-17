#include "HeuristicOptimizer.h"
#include "Coordinator.h"
#include "Display.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class TranscodeOptimizerTestFixture : public testing::Test {
public:
    TranscodeOptimizerTestFixture()
            : catalog("resources")
    { Catalog::instance(catalog); }

    void testGOP(const size_t gop) {
        auto source = Scan("red10"); //std::string("test" + std::to_string(gop)));
        auto encoded = source.Encode(Codec::hevc());

        auto environment = LocalEnvironment();
        auto coordinator = Coordinator();

        Plan plan = HeuristicOptimizer(environment).optimize(encoded);

        print_plan(plan);

        coordinator.save(plan, "out.hevc");
    }

protected:
    Catalog catalog;
};

TEST_F(TranscodeOptimizerTestFixture, testGOP30) {
    testGOP(30);
}

TEST_F(TranscodeOptimizerTestFixture, testGOP15) {
    testGOP(15);
}


TEST_F(TranscodeOptimizerTestFixture, testTranscode) {
    auto source = Scan("red10");
    auto encoded = source.Encode(Codec::hevc());

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();

    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    print_plan(plan);

    coordinator.save(plan, "out.hevc");
}
