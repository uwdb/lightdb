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
            : catalog(Resources.catalog_name) {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

protected:
    Catalog catalog;
};

TEST_F(UDFTestFixture, testGreyscale) {
    auto query = Scan(Resources.red10.name)
                    .Map(lightdb::Greyscale)
                    .Encode()
                    .Save("/home/maureen/grey_test.hevc");

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(UDFTestFixture, testLoadedBlur) {
    auto blur = lightdb::extensibility::Load(Resources.plugins.blur.name);

    auto query = Scan(Resources.red10.name)
                    .Map(blur)
                    .Encode()
                    .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}
