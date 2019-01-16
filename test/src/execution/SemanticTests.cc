#include "Operators.h"
#include "PhysicalOperators.h"
#include "HeuristicOptimizer.h"
#include "Catalog.h"
#include "AssertVideo.h"
#include "Display.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::catalog;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class SemanticTestFixture : public testing::Test {
public:
    SemanticTestFixture()
            : catalog("resources") {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

    Volume GetSinglePixelVolume(const LightFieldReference& ref) {
        auto volume = GetSingleFrameVolume(ref);
        volume.theta({0, pixel.first});
        volume.phi({0, pixel.second});
        return volume;
    }

    Volume GetSingleFrameVolume(const LightFieldReference& ref) {
        auto volume = ref->volume().bounding();
        volume.t({0, framerate});
        return volume;
    }

protected:
    Catalog catalog;
    const rational framerate = {1, 25};
    const std::pair<rational_times_real, rational_times_real> pixel = {{{2, 320}, PI}, {{1, 240}, PI}};
};

TEST_F(SemanticTestFixture, testDecode) {
    //auto video = Decode("resources/red10.h264").apply();
    //auto *discrete = dynamic_cast<PanoramicVideoLightField*>(&*video);
    auto input = "red10";
    auto output = "out.h264";

    auto l = Scan(input);
    auto e = l.Encode();

    auto plan = Optimizer::instance().optimize(e);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_MEAN_RGB(output, l.downcast<ScannedLightField>().metadata().streams()[0].path().string(), 0);
    //EXPECT_EQ(remove(output), 0);

    //ASSERT_EQ(result.size(), 1);
    //ASSERT_EQ(result.front(), YUVColor::red());

    //printf("%d\n", output.str().size());
    //printf("%d\n", output.str()[0]);

    //TODO fix this after adding rational overloads
    //auto fps = (long double)discrete->framerate(); //.numerator() / discrete->framerate().denominator();

    //TODO
    /*ASSERT_EQ(video->value({0, 0, 0, 0,     0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, fps*5, 0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, fps/2, 0, 0}), YUVColor::Null);
    ASSERT_EQ(video->value({0, 0, 0, 99,    0, 0}), YUVColor::Null);*/
}

TEST_F(SemanticTestFixture, testUnion) {
    auto red = Decode("resources/red10.h264").apply();
    auto green = ConstantLightField::create(YUVColor::green());

    //TODO
    /*ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);*/

    auto combined = red | green;

    //TODO
    /*ASSERT_EQ(combined->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(combined->value({0, 0, 0, 99, 0, 0}), YUVColor::Green);*/
}

TEST_F(SemanticTestFixture, testSelect) {
    auto green = ConstantLightField::create(YUVColor::green());

    //TODO
    /*ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(green->value({0, 0, 0, 99, 0, 0}), YUVColor::Green);*/

    auto selected = green >> Select(Volume{Point3D::zero(), {0, 1}, ThetaRange::limits(), PhiRange::limits()});

    //TODO
    /*ASSERT_EQ(selected->value({0, 0, 0, 0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(selected->value({0, 0, 0, 2, 0, 0}), YUVColor::Null);*/
}

TEST_F(SemanticTestFixture, testUnionEncode) {
    auto red = Decode("resources/red10.h264").apply();
    auto green = Decode("resources/green10.h264").apply();

    //TODO
    /*ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);
    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);
    ASSERT_EQ(green->value({0, 0, 0, 99, 0, 0}), YUVColor::Null);*/

    auto result = (red | green) >> Encode();

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10-green10.h264", Volume::limits())->bytes());
}

TEST_F(SemanticTestFixture, testIdentityEncode) {
    auto video = Decode("resources/red10.h264").apply();

    //TODO
    /*ASSERT_EQ(video->value({0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(video->value({0, 0, 0, 99, 0, 0}), YUVColor::Null);*/

    auto result = video >> Encode("h264");

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10.h264", Volume::limits())->bytes());
}

TEST_F(SemanticTestFixture, testUnionSelect) {
    auto red = Decode("resources/red10.h264").apply();
    auto green = ConstantLightField::create(YUVColor::green());

    //TODO
    /*ASSERT_EQ(red->value(  {0, 0, 0,  0, 0, 0}), YUVColor::Red);
    ASSERT_EQ(red->value(  {0, 0, 0, 99, 0, 0}), YUVColor::Null);
    ASSERT_EQ(green->value({0, 0, 0,  0, 0, 0}), YUVColor::Green);*/

    auto result = (red | green)
            >> Select({Point3D::zero(), {0, 20}, ThetaRange::limits(), PhiRange::limits()})
            >> Encode();

    ASSERT_GT(result->bytes()->size(), 0);
    ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10-green10.h264", Volume::limits())->bytes());
}
