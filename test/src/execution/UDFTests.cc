#include "HeuristicOptimizer.h"
#include "Greyscale.h"
#include "extension.h"
#include "Display.h"
#include "AssertVideo.h"
#include "TestResources.h"

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class UDFTestFixture : public testing::Test {
public:
    UDFTestFixture()
            : catalog("resources") {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

protected:
    Catalog catalog;
};

TEST_F(UDFTestFixture, testGreyscale) {
    auto name = "red10";
    auto output = "out.hevc";

    auto input = Scan(name);
    auto gray = input.Map(lightdb::Greyscale);
    auto encoded = gray.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height, Resources.red10.width);
    EXPECT_EQ(remove(output), 0);
}

TEST_F(UDFTestFixture, testBlur) {
    auto name = "red10";
    auto output = "out.hevc";

    auto blur = lightdb::extensibility::Load("blur");

    auto input = Scan(name);
    auto annotated = input.Map(blur);
    auto encoded = annotated.Encode();

    auto plan = Optimizer::instance().optimize(encoded);
    Coordinator().save(plan, output);

    EXPECT_VIDEO_VALID(output);
    EXPECT_VIDEO_FRAMES(output, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(output, Resources.red10.height, Resources.red10.width);
    //EXPECT_EQ(remove(output), 0);
}
