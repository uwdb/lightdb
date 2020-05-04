#include "HeuristicOptimizer.h"
#include "Display.h"
#include "TestResources.h"
#include "AssertVideo.h"
#include "AssertTime.h"
#include "AssertUtility.h"

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class SelectionTestFixture : public testing::Test {
public:
    SelectionTestFixture()
            : catalog(Resources.catalog_name) {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

protected:
    Catalog catalog;
};

TEST_F(SelectionTestFixture, testEmptySelection) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                     .Select(Point6D::zero())
                     .Encode(Codec::raw());

    ASSERT_EQ(Coordinator().serialize(query), "");
}

TEST_F(SelectionTestFixture, testSelectThetaPhi) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                    .Select(ThetaRange{0, rational_times_real({2, 4}, PI)})
                    .Select(PhiRange{0, rational_times_real({1, 4}, PI)})
                    .Encode()
                    .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height / 4, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(SelectionTestFixture, testSelectPhiTheta) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                    .Select(PhiRange{0, rational_times_real({1, 4}, PI)})
                    .Select(ThetaRange{0, rational_times_real({2, 4}, PI)})
                    .Encode()
                    .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height / 4, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(SelectionTestFixture, testSelectPhi) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                    .Select(PhiRange{0, rational_times_real({1, 4}, PI)})
                    .Encode()
                    .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height / 4, Resources.red10.width);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(SelectionTestFixture, testSelectTheta) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                     .Select(ThetaRange{0, rational_times_real({2, 4}, PI)})
                     .Encode()
                     .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(SelectionTestFixture, testTemporalSelect) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                     .Select(TemporalRange{2, 5})
                     .Encode()
                     .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.framerate * 3);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(SelectionTestFixture, testThetaTemporalSelect) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                    .Select(ThetaRange{0, rational_times_real({2, 4}, PI)})
                    .Select(SpatiotemporalDimension::Time, TemporalRange{2, 5})
                    .Encode()
                    .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.framerate * 3);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(SelectionTestFixture, testTemporalThetaSelect) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                    .Select(SpatiotemporalDimension::Time, TemporalRange{2, 5})
                    .Select(ThetaRange{0, rational_times_real({2, 4}, PI)})
                    .Encode()
                    .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.framerate * 3);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(SelectionTestFixture, testDegenerateTimeSelect) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
            .Select(TemporalRange::limits())
            .Encode()
            .Save(Resources.out.h264);

    LOG_DURATION(Resources.red10.name,
                 ASSERT_MSECS(
                         Coordinator().execute(query),
                         400u));

    EXPECT_VIDEO_VALID(Resources.out.h264);
    EXPECT_VIDEO_FRAMES(Resources.out.h264, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.h264, Resources.red10.height, Resources.red10.width);
    EXPECT_EQ(remove(Resources.out.h264), 0);
}

TEST_F(SelectionTestFixture, testDegenerateAngularSelect) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
            .Select(ThetaRange::limits())
            .Encode()
            .Save(Resources.out.h264);

    LOG_DURATION(Resources.red10.name,
                 ASSERT_MSECS(
                         Coordinator().execute(query),
                         400u));

    EXPECT_VIDEO_VALID(Resources.out.h264);
    EXPECT_VIDEO_FRAMES(Resources.out.h264, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.h264, Resources.red10.height, Resources.red10.width);
    EXPECT_EQ(remove(Resources.out.h264), 0);
}
