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
    auto input = Scan(Resources.red10.name).Save(Resources.out.raw);
    Coordinator().execute(input);

    auto output_h264 = TRANSCODE_RAW_TO_H264(Resources.out.raw,
            Resources.red10.height, Resources.red10.width,
            Resources.red10.framerate);

    EXPECT_VIDEO_VALID(output_h264);
    EXPECT_VIDEO_FRAMES(output_h264, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output_h264, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(output_h264);
    EXPECT_EQ(remove(Resources.out.raw), 0);
    EXPECT_EQ(remove(output_h264.c_str()), 0);
}