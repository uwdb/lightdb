#include "HeuristicOptimizer.h"
#include "Catalog.h"
#include "AssertVideo.h"
#include "Display.h"
#include "TestResources.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::catalog;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class DecodeTestFixture : public testing::Test {
public:
    DecodeTestFixture()
            : catalog(Resources.catalog_name) {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

protected:
    Catalog catalog;
};

TEST_F(DecodeTestFixture, testDecode) {
    auto output = "out.raw";

    auto input = Scan(Resources.red10.name);

    auto plan = Optimizer::instance().optimize(input);
    Coordinator().save(plan, output);

    auto output_h264 = TRANSCODE_RAW_TO_H264(output,
            Resources.red10.height, Resources.red10.width,
            Resources.red10.framerate);

    EXPECT_VIDEO_VALID(output_h264);
    EXPECT_VIDEO_FRAMES(output_h264, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output_h264, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(output_h264);
    EXPECT_EQ(remove(output), 0);
    EXPECT_EQ(remove(output_h264.c_str()), 0);
}