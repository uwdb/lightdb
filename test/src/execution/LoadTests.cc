#include "HeuristicOptimizer.h"
#include "Catalog.h"
#include "AssertVideo.h"
#include "Display.h"
#include "TestResources.h"
#include <gtest/gtest.h>
#include <TestResources.h>

using namespace lightdb;
using namespace lightdb::catalog;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class LoadTestFixture : public testing::Test {
public:
    LoadTestFixture() {
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }
};

TEST_F(LoadTestFixture, testLoadMP4WithoutMetadata) {
    auto query = Load(Resources.videos.black.mp4.name,
                      {{GeometryOptions::Volume, Volume::zero()},
                      {GeometryOptions::Projection, GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples())}})
                     .Save(Resources.out.h264);

    EXPECT_EQ(query->volume().bounding().t().magnitude(), Resources.videos.black.duration);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.h264);
    EXPECT_VIDEO_FRAMES(Resources.out.h264, Resources.videos.black.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.h264, Resources.videos.black.height, Resources.videos.black.width);
    EXPECT_VIDEO_BLACK(Resources.out.h264);
    EXPECT_EQ(remove(Resources.out.h264), 0);
}

TEST_F(LoadTestFixture, testLoadMP4WithMetadata) {
    auto query = Load(Resources.red10.metadata_path).Save(Resources.out.h264);

    EXPECT_EQ(query->volume().bounding().t(),
              Scan(Catalog(Resources.catalog_name), Resources.red10.name)->volume().bounding().t());

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.h264);
    EXPECT_VIDEO_FRAMES(Resources.out.h264, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.h264, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(Resources.out.h264);
    EXPECT_EQ(remove(Resources.out.h264), 0);
}

TEST_F(LoadTestFixture, testLoadUnmuxed) {
    auto query = Load(Resources.videos.black.h264.name,
                      {{GeometryOptions::Volume, Volume::zero()},
                       {GeometryOptions::Projection, GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples())}})
                     .Save(Resources.out.h264);

    EXPECT_EQ(query->volume().bounding().t().magnitude(), Resources.videos.black.duration);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.h264);
    EXPECT_VIDEO_FRAMES(Resources.out.h264, Resources.videos.black.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.h264, Resources.videos.black.height, Resources.videos.black.width);
    EXPECT_VIDEO_BLACK(Resources.out.h264);
    EXPECT_EQ(remove(Resources.out.h264), 0);
}

TEST_F(LoadTestFixture, testLoadUnmuxedDifferentExtension) {
    auto filename = Resources.videos.black.h264.name + "foo";
    std::filesystem::copy(Resources.videos.black.h264.name, filename);

    auto query = Load(filename,
                      {{GeometryOptions::Volume, Volume::zero()},
                       {GeometryOptions::Projection, GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples())}})
            .Save(Resources.out.h264);

    EXPECT_EQ(query->volume().bounding().t().magnitude(), Resources.videos.black.duration);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.h264);
    EXPECT_VIDEO_FRAMES(Resources.out.h264, Resources.videos.black.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.h264, Resources.videos.black.height, Resources.videos.black.width);
    EXPECT_VIDEO_BLACK(Resources.out.h264);
    EXPECT_EQ(remove(Resources.out.h264), 0);
    EXPECT_EQ(remove(filename.c_str()), 0);
}
