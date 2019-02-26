#include "HeuristicOptimizer.h"
#include "Coordinator.h"
#include "Display.h"
#include "TestResources.h"
#include "AssertVideo.h"
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

    void testGOP(const unsigned int gop) {
        auto source = Scan(Resources.red10.name);
        auto encoded = source.Encode(Codec::hevc(), {{"GOP", gop}});

        auto environment = LocalEnvironment();
        auto coordinator = Coordinator();

        Plan plan = HeuristicOptimizer(environment).optimize(encoded);

        coordinator.save(plan, Resources.out.hevc);

        EXPECT_VIDEO_VALID(Resources.out.hevc);
        EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
        EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width);
        EXPECT_VIDEO_RED(Resources.out.hevc);
        EXPECT_VIDEO_GOP(Resources.out.hevc, gop);
    }

protected:
    Catalog catalog;
};

TEST_F(TranscodeOptimizerTestFixture, testGOP30) {
    testGOP(30u);
}

TEST_F(TranscodeOptimizerTestFixture, testGOP15) {
    testGOP(15u);
}

TEST_F(TranscodeOptimizerTestFixture, testGOP7) {
    testGOP(7u);
}

TEST_F(TranscodeOptimizerTestFixture, testImplicitGOP) {
    auto source = Scan(Resources.red10.name);
    auto encoded = source.Encode(Codec::hevc());

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();

    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    coordinator.save(plan, Resources.out.hevc);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_VIDEO_GOP(Resources.out.hevc, physical::GPUEncode::kDefaultGopSize);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}
