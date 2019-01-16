#include "HeuristicOptimizer.h"
#include "Display.h"
#include "AssertVideo.h"

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class SelectionTestFixture : public testing::Test {
public:
    SelectionTestFixture()
            : catalog("resources") {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

protected:
    Catalog catalog;
};

TEST_F(SelectionTestFixture, testEmptySelection) {
    auto input = Scan("red10");
    auto zero = input.Select(Point6D::zero());
    auto encoded = zero.Encode(Codec::raw());

    auto plan = Optimizer::instance().optimize(encoded);

    ASSERT_EQ(Coordinator().save(plan), "");
}

TEST_F(SelectionTestFixture, testSelectThetaPhi) {
    auto name = "red10";
    auto output = "out.hevc";

    auto input = Scan(name);
    auto theta = input.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto phi = theta.Select(PhiRange{0, rational_times_real({1, 4}, PI)});
    auto encoded = phi.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, 250);
    EXPECT_VIDEO_RESOLUTION(output, 240 / 4, 320 / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testSelectPhiTheta) {
    auto name = "red10";
    auto output = "out.hevc";

    auto input = Scan(name);
    auto phi = input.Select(PhiRange{0, rational_times_real({1, 4}, PI)});
    auto theta = phi.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto encoded = theta.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, 250);
    EXPECT_VIDEO_RESOLUTION(output, 240 / 4, 320 / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testSelectPhi) {
    auto name = "red10";
    auto output = "out.hevc";

    auto input = Scan(name);
    auto phi = input.Select(PhiRange{0, rational_times_real({1, 4}, PI)});
    auto encoded = phi.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, 250);
    EXPECT_VIDEO_RESOLUTION(output, 240 / 4, 320);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testSelectTheta) {
    auto name = "red10";
    auto output = "out.hevc";

    auto input = Scan(name);
    auto theta = input.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto encoded = theta.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, 250);
    EXPECT_VIDEO_RESOLUTION(output, 240, 320 / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testTemporalSelect) {
    auto name = "red10";
    auto output = "out.hevc";

    auto input = Scan(name);
    auto temporal = input.Select(SpatiotemporalDimension::Time, TemporalRange{2, 5});
    auto encoded = temporal.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, 25 * 3);
    EXPECT_VIDEO_RESOLUTION(output, 240, 320);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testThetaTemporalSelect) {
    auto name = "red10";
    auto output = "out.hevc";

    auto input = Scan(name);
    auto theta = input.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto temporal = theta.Select(SpatiotemporalDimension::Time, TemporalRange{2, 5});
    auto encoded = temporal.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, 25 * 3);
    EXPECT_VIDEO_RESOLUTION(output, 240, 320 / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(SelectionTestFixture, testTemporalThetaSelect) {
    auto name = "red10";
    auto output = "out.hevc";

    auto input = Scan(name);
    auto temporal = input.Select(SpatiotemporalDimension::Time, TemporalRange{2, 5});
    auto theta = temporal.Select(ThetaRange{0, rational_times_real({2, 4}, PI)});
    auto encoded = theta.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, 25 * 3);
    EXPECT_VIDEO_RESOLUTION(output, 240, 320 / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}



