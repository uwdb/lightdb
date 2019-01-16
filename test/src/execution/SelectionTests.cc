#include "HeuristicOptimizer.h"
#include "Display.h"
#include "TestResources.h"
#include "AssertVideo.h"

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
    auto input = Scan(Resources.red10.name);
    auto zero = input.Select(Point6D::zero());
    auto encoded = zero.Encode(Codec::raw());

    auto plan = Optimizer::instance().optimize(encoded);

    ASSERT_EQ(Coordinator().save(plan), "");
}

TEST_F(SelectionTestFixture, testSelectThetaPhi) {
    auto output = "out.hevc";

    auto input = Scan(Resources.red10.name);
    auto theta = input.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto phi = theta.Select(PhiRange{0, rational_times_real({1, 4}, PI)});
    auto encoded = phi.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height / 4, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testSelectPhiTheta) {
    auto output = "out.hevc";

    auto input = Scan(Resources.red10.name);
    auto phi = input.Select(PhiRange{0, rational_times_real({1, 4}, PI)});
    auto theta = phi.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto encoded = theta.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height / 4, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testSelectPhi) {
    auto output = "out.hevc";

    auto input = Scan(Resources.red10.name);
    auto phi = input.Select(PhiRange{0, rational_times_real({1, 4}, PI)});
    auto encoded = phi.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height / 4, Resources.red10.width);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testSelectTheta) {
    auto output = "out.hevc";

    auto input = Scan(Resources.red10.name);
    auto theta = input.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto encoded = theta.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testTemporalSelect) {
    auto output = "out.hevc";

    auto input = Scan(Resources.red10.name);
    auto temporal = input.Select(SpatiotemporalDimension::Time, TemporalRange{2, 5});
    auto encoded = temporal.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.framerate * 3);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testThetaTemporalSelect) {
    auto output = "out.hevc";

    auto input = Scan(Resources.red10.name);
    auto theta = input.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto temporal = theta.Select(SpatiotemporalDimension::Time, TemporalRange{2, 5});
    auto encoded = temporal.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.framerate * 3);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testTemporalThetaSelect) {
    auto output = "out.hevc";

    auto input = Scan(Resources.red10.name);
    auto temporal = input.Select(SpatiotemporalDimension::Time, TemporalRange{2, 5});
    auto theta = temporal.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto encoded = theta.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.framerate * 3);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}



