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
using namespace lightdb::physical;
using namespace lightdb::errors;

class EncodeTestFixture : public testing::Test {
public:
    EncodeTestFixture()
            : catalog(Resources.catalog_name) {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

    void testEncodeGOP(const unsigned int gop) {
        auto query = Scan(Resources.red10.name)
                          .Encode(Codec::hevc(), {{EncodeOptions::GOPSize, gop}})
                          .Save(Resources.out.hevc);

        Coordinator().execute(query);

        EXPECT_VIDEO_VALID(Resources.out.hevc);
        EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
        EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width);
        EXPECT_VIDEO_RED(Resources.out.hevc);
        EXPECT_VIDEO_GOP(Resources.out.hevc, gop);
    }

protected:
    Catalog catalog;
};

TEST_F(EncodeTestFixture, testEncodeH264) {
    REQUIRES_GPU();

    auto query = Scan(Resources.red10.name)
                     .Encode(Codec::h264())
                     .Save(Resources.out.h264);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.h264);
    EXPECT_VIDEO_FRAMES(Resources.out.h264, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.h264, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(Resources.out.h264);
    EXPECT_EQ(remove(Resources.out.h264), 0);
}

TEST_F(EncodeTestFixture, testEncodeHEVC) {
    REQUIRES_GPU();

    auto query = Scan(Resources.red10.name)
                     .Encode(Codec::hevc())
                     .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(EncodeTestFixture, testEncodeRaw) {
    REQUIRES_GPU();

    auto query = Scan(Resources.red10.name)
                     .Encode(Codec::raw())
                     .Save(Resources.out.raw);

    Coordinator().execute(query);

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

TEST_F(EncodeTestFixture, testGOP30) {
    REQUIRES_GPU();

    testEncodeGOP(30u);
}

TEST_F(EncodeTestFixture, testGOP15) {
    REQUIRES_GPU();

    testEncodeGOP(15u);
}

TEST_F(EncodeTestFixture, testGOP7) {
    REQUIRES_GPU();

    testEncodeGOP(7u);
}

TEST_F(EncodeTestFixture, testImplicitGOP) {
    REQUIRES_GPU();

    auto query = Scan(Resources.red10.name)
                     .Encode(Codec::hevc())
                     .Save(Resources.out.hevc);

    Coordinator().execute(query);

    EXPECT_VIDEO_VALID(Resources.out.hevc);
    EXPECT_VIDEO_FRAMES(Resources.out.hevc, Resources.red10.frames);
    EXPECT_VIDEO_RESOLUTION(Resources.out.hevc, Resources.red10.height, Resources.red10.width);
    EXPECT_VIDEO_RED(Resources.out.hevc);
    EXPECT_VIDEO_GOP(Resources.out.hevc, physical::GPUEncodeToCPU::kDefaultGopSize);
    EXPECT_EQ(remove(Resources.out.hevc), 0);
}

TEST_F(EncodeTestFixture, testInvalidGOPType) {
    auto query = Scan(Resources.red10.name)
                     .Encode(Codec::hevc(), {{EncodeOptions::GOPSize, "invalid"}})
                     .Save(Resources.out.hevc);

    EXPECT_THROW(Coordinator().execute(query), _InvalidArgument);
}

TEST_F(EncodeTestFixture, testInvalidGOPRange) {
    auto query = Scan(Resources.red10.name)
                     .Encode(Codec::hevc(), {{EncodeOptions::GOPSize, -1}})
                     .Save(Resources.out.hevc);

    EXPECT_THROW(Coordinator().execute(query), _InvalidArgument);
}
