#include "Tiler.h"
#include "AssertTime.h"
#include "AssertVideo.h"

#define FILENAME(n) (std::string("resources/test-pattern-") + std::to_string(n) + ".h265")

class TilerVideoEncoderTestFixture : public testing::Test {
public:
    TilerVideoEncoderTestFixture()
        : context(0),
          configuration(1080, 1920, 2, 2, NV_ENC_HEVC, "hq", 24, 30, 1024*1024, 0, 0)
    { }

protected:
    GPUContext context;
    EncodeConfig configuration;
};


TEST_F(TilerVideoEncoderTestFixture, testConstructor) {
}


TEST_F(TilerVideoEncoderTestFixture, testSingleTile) {
    TileVideoEncoder2 tiler(context, configuration, 1, 1);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers;

    writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(0)));

    ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), configuration.height, configuration.width);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(TilerVideoEncoderTestFixture, testTwoRows) {
    TileVideoEncoder2 tiler(context, configuration, 2, 1);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers{
        std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(0)),
        std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(1))
    };

    ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_VALID(FILENAME(1));

    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_FRAMES(FILENAME(1), 99);

    EXPECT_VIDEO_RESOLUTION(FILENAME(0), configuration.height / 2, configuration.width);
    EXPECT_VIDEO_RESOLUTION(FILENAME(1), configuration.height / 2, configuration.width);

    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
    EXPECT_EQ(remove(FILENAME(1).c_str()), 0);
}

TEST_F(TilerVideoEncoderTestFixture, testTwoColumns) {
    TileVideoEncoder2 tiler(context, configuration, 1, 2);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers {
        std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(0)),
        std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(1))
    };

    ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_VALID(FILENAME(1));

    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_FRAMES(FILENAME(1), 99);

    EXPECT_VIDEO_RESOLUTION(FILENAME(0), configuration.height, configuration.width / 2);
    EXPECT_VIDEO_RESOLUTION(FILENAME(1), configuration.height, configuration.width / 2);

    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
    EXPECT_EQ(remove(FILENAME(1).c_str()), 0);
}

TEST_F(TilerVideoEncoderTestFixture, test2x2) {
    const auto rows = 2, columns = 2;
    TileVideoEncoder2 tiler(context, configuration, rows, columns);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers;

    for(auto i = 0; i < rows * columns; i++)
        writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(i)));

    ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS);

    for(auto i = 0; i < rows * columns; i++) {
        EXPECT_VIDEO_VALID(FILENAME(i));
        EXPECT_VIDEO_FRAMES(FILENAME(i), 99);
        EXPECT_VIDEO_RESOLUTION(FILENAME(i), configuration.height / rows, configuration.width / columns);
        EXPECT_EQ(remove(FILENAME(i).c_str()), 0);
    }
}

TEST_F(TilerVideoEncoderTestFixture, test6x2) {
    const auto rows = 6, columns = 2;
    TileVideoEncoder2 tiler(context, configuration, rows, columns);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers;

    for(auto i = 0; i < rows * columns; i++)
        writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(i)));

    ASSERT_SECS(
        ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS),
        0.25);

    for(auto i = 0; i < rows * columns; i++) {
        EXPECT_VIDEO_VALID(FILENAME(i));
        EXPECT_VIDEO_FRAMES(FILENAME(i), 99);
        EXPECT_VIDEO_RESOLUTION(FILENAME(i), configuration.height / rows, configuration.width / columns);
        EXPECT_EQ(remove(FILENAME(i).c_str()), 0);
    }
}

TEST_F(TilerVideoEncoderTestFixture, test2x8) {
    const auto rows = 2, columns = 8;
    TileVideoEncoder2 tiler(context, configuration, rows, columns);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers;

    for(auto i = 0; i < rows * columns; i++)
        writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(i)));

    ASSERT_SECS(
            ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS),
        0.25);

    for(auto i = 0; i < rows * columns; i++) {
        EXPECT_VIDEO_VALID(FILENAME(i));
        EXPECT_VIDEO_FRAMES(FILENAME(i), 99);
        EXPECT_VIDEO_RESOLUTION(FILENAME(i), configuration.height / rows, configuration.width / columns);
        EXPECT_EQ(remove(FILENAME(i).c_str()), 0);
    }
}

TEST_F(TilerVideoEncoderTestFixture, test1x8) {
    const auto rows = 1, columns = 8;
    TileVideoEncoder2 tiler(context, configuration, rows, columns);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers;

    for(auto i = 0; i < rows * columns; i++)
        writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(i)));

    ASSERT_SECS(
            ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS),
            0.25);

    for(auto i = 0; i < rows * columns; i++) {
        EXPECT_VIDEO_VALID(FILENAME(i));
        EXPECT_VIDEO_FRAMES(FILENAME(i), 99);
        EXPECT_VIDEO_RESOLUTION(FILENAME(i), configuration.height / rows, configuration.width / columns);
        EXPECT_EQ(remove(FILENAME(i).c_str()), 0);
    }
}

TEST_F(TilerVideoEncoderTestFixture, test6x1) {
    const auto rows = 6, columns = 1;
    TileVideoEncoder2 tiler(context, configuration, rows, columns);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers;

    for(auto i = 0; i < rows * columns; i++)
        writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(i)));

    ASSERT_SECS(
            ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS),
            0.25);

    for(auto i = 0; i < rows * columns; i++) {
        EXPECT_VIDEO_VALID(FILENAME(i));
        EXPECT_VIDEO_FRAMES(FILENAME(i), 99);
        EXPECT_VIDEO_RESOLUTION(FILENAME(i), configuration.height / rows, configuration.width / columns);
        EXPECT_EQ(remove(FILENAME(i).c_str()), 0);
    }
}

TEST_F(TilerVideoEncoderTestFixture, test6x8) {
    const auto rows = 6, columns = 8;
    TileVideoEncoder2 tiler(context, configuration, rows, columns);
    FileDecodeReader reader("resources/test-pattern.h264");
    std::vector<std::shared_ptr<EncodeWriter>> writers;

    for(auto i = 0; i < rows * columns; i++)
        writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), FILENAME(i)));

    ASSERT_SECS(
            ASSERT_EQ(tiler.tile(reader, writers), NV_ENC_SUCCESS),
            0.75);

    for(auto i = 0; i < rows * columns; i++) {
        EXPECT_VIDEO_VALID(FILENAME(i));
        EXPECT_VIDEO_FRAMES(FILENAME(i), 99);
        EXPECT_VIDEO_RESOLUTION(FILENAME(i), configuration.height / rows, configuration.width / columns);
        EXPECT_EQ(remove(FILENAME(i).c_str()), 0);
    }
}

TEST_F(TilerVideoEncoderTestFixture, testOddTileSize) {
    const auto rows = 8, columns = 1;
    EXPECT_ANY_THROW(TileVideoEncoder2(context, configuration, rows, columns));
}