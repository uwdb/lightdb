#include "HeuristicOptimizer.h"
#include "Display.h"
#include "TestResources.h"
#include "AssertVideo.h"
#include "AssertUtility.h"

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class UnionTestFixture : public testing::Test {
public:
    UnionTestFixture()
            : catalog(Resources.catalog_name) {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

protected:
    Catalog catalog;
};

TEST_F(UnionTestFixture, testEmptyUnion) {
    REQUIRE_GPU();

    auto query = Scan(Resources.red10.name)
                    .Select(Point6D::zero())
                    .Encode(Codec::raw());

    ASSERT_EQ(Coordinator().serialize(query), "");
}

/*
TEST_F(UnionTestFixture, testSelfUnion) {
}

TEST_F(UnionTestFixture, testOverlappingUnion) {
    auto output = "out.hevc";

    auto red = Scan(Resources.red10.name);
    auto green = Scan(Resources.green10.name);
    auto unioned = red.Union(green);
    auto encoded = unioned.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    print_plan(plan);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height / 4, Resources.red10.width / 4);
    EXPECT_VIDEO_RED(output);
    EXPECT_EQ(remove(output), 0);
}
*/

/*
TEST_F(SemanticTestFixture, testUnion) {
    auto red = Decode("resources/red10.h264").apply();
    auto green = ConstantLightField::create(YUVColor::green());

    //TODO
    //ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    //ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    //ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);

    auto combined = red | green;

    //TODO
    //ASSERT_EQ(combined->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    //ASSERT_EQ(combined->value({0, 0, 0, 99, 0, 0}), YUVColor::Green);
}

TEST_F(SemanticTestFixture, testUnionSelect) {
    auto red = Decode("resources/red10.h264").apply();
    auto green = ConstantLightField::create(YUVColor::green());

    //TODO
    //ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    //ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);
    //ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);

    auto result = (red | green)
            >> Select({Point3D::zero(), {0, 20}, ThetaRange::limits(), PhiRange::limits()})
            >> Encode();

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10-green10.h264", Volume::limits())->bytes());
}
*/
