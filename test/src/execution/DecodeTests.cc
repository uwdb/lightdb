#include "HeuristicOptimizer.h"
#include "Catalog.h"
#include "AssertVideo.h"
#include "Display.h"
#include "TestResources.h"
#include "AssertUtility.h"
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
    //REQUIRE_GPU();

    auto input = Scan(Resources.red10.name).Save(Resources.out.raw);
    LOG(ERROR) << "foo0";
    Coordinator().execute(input);
LOG(ERROR) << "foo1";
    auto output_h264 = TRANSCODE_RAW_TO_H264(Resources.out.raw,
            Resources.red10.height, Resources.red10.width,
            Resources.red10.framerate);
    LOG(ERROR) << "foo2";

    EXPECT_VIDEO_VALID(output_h264);
    LOG(ERROR) << "foo3";
    EXPECT_VIDEO_FRAMES(output_h264, Resources.red10.frames);
    LOG(ERROR) << "foo4";
    EXPECT_VIDEO_RESOLUTION(output_h264, Resources.red10.height, Resources.red10.width);
    LOG(ERROR) << "foo5";
    EXPECT_VIDEO_RED(output_h264);
    LOG(ERROR) << "foo6";
    EXPECT_EQ(remove(Resources.out.raw), 0);
    LOG(ERROR) << "foo7";
    EXPECT_EQ(remove(output_h264.c_str()), 0);
    LOG(ERROR) << "foo8";
}

TEST_F(DecodeTestFixture, testDecodeCPU) {
    auto cpu_optimizer = OptimizerReference::make<HeuristicOptimizer>(LocalEnvironment(false));

    auto input = Scan(Resources.red10.name).Save(Resources.out.raw);
    Coordinator().execute(input, *cpu_optimizer);

    auto output_h264 = TRANSCODE_RAW_TO_H264(Resources.out.raw,
                                             Resources.red10.height, Resources.red10.width,
                                             Resources.red10.framerate, "yuv420p");

    EXPECT_VIDEO_VALID(output_h264);
    EXPECT_VIDEO_FRAMES(output_h264, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output_h264, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(output_h264);
    EXPECT_EQ(remove(Resources.out.raw), 0);
    EXPECT_EQ(remove(output_h264.c_str()), 0);
}


/*
TEST_F(DecodeTestFixture, testDecodeMP4) {
    auto input = Load(Resources.videos.black.mp4.name,
                      {{"Codec", Resources.videos.black.mp4.codec}})
                     .Save(Resources.out.h264);
    Coordinator().execute(input);

    EXPECT_VIDEO_VALID(Resources.out.h264);
    EXPECT_VIDEO_FRAMES(Resources.out.h264, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.h264, Resources.videos.black.mp4.height, Resources.videos.black.mp4.width);
    EXPECT_VIDEO_RED(Resources.out.h264);
    EXPECT_EQ(remove(Resources.out.h264), 0);
}*/
