#include "JoinVideoEncoder.h"
#include "AssertTime.h"
#include "AssertVideo.h"

#define FILENAME(n) (std::string("resources/test-pattern-") + std::to_string(n) + ".h265")

class JoinVideoEncoderTestFixture : public testing::Test {
public:
    JoinVideoEncoderTestFixture()
        : context(0),
          decodeConfiguration1K(1024, 512,  24, 1024*1024, cudaVideoCodec_H264),
          decodeConfiguration2K(1920, 1080, 24, 1024*1024, cudaVideoCodec_H264),
          decodeConfiguration4K(3840, 2160, 24, 1024*1024, cudaVideoCodec_H264)
    { }

protected:
    GPUContext context;
    DecodeConfiguration decodeConfiguration1K;
    DecodeConfiguration decodeConfiguration2K;
    DecodeConfiguration decodeConfiguration4K;
};


TEST_F(JoinVideoEncoderTestFixture, testConstructor) {
}

TEST_F(JoinVideoEncoderTestFixture, testSingleTile) {
    const auto height = 1080, width = 1920;
    const auto inputFilename = "resources/test-pattern.h264";

    EncodeConfiguration encodeConfiguration(1080, 1920, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {decodeConfiguration2K}, encodeConfiguration, 1, 1);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};

    readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR, 0, 0, width, height);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(JoinVideoEncoderTestFixture, testTwoRows) {
    const auto height = 1080, width = 1920;
    auto inputFilename = "resources/test-pattern.h264";

    EncodeConfiguration encodeConfiguration(2*height, width, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {2, decodeConfiguration2K}, encodeConfiguration, 2, 1);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};

    readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));
    readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);

    // Verify top half
    EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR,
                         0, 0, width, height,
                         0, 0, width, height);
    // Verify bottom half
    EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR,
                         0, 0, width, height,
                         0, height, width, height * 2);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(JoinVideoEncoderTestFixture, testTwoColumns) {
    const auto height = 1080, width = 1920;
    const auto inputFilename = "resources/test-pattern.h264";

    EncodeConfiguration encodeConfiguration(height, 2*width, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {2, decodeConfiguration2K}, encodeConfiguration, 1, 2);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};

    readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));
    readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);

    // Verify left half
    EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR,
                         0, 0, width, height,
                         0, 0, width, height);
    // Verify right half
    EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR,
                         0, 0, width, height,
                         width, 0, width * 2, height);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(JoinVideoEncoderTestFixture, test2x2) {
    const auto rows = 2u, columns = 2u;
    const auto height = 1080u, width = 1920u;
    const auto inputFilename = "resources/test-pattern.h264";

    EncodeConfiguration encodeConfiguration(rows*height, columns*width, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {rows*columns, decodeConfiguration2K}, encodeConfiguration, rows, columns);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};

    for(auto i = 0u; i < rows*columns; i++)
       readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);

    for(auto x = 0u; x < columns; x++)
        for(auto y = 0u; y < rows; y++)
            EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR,
                                 // Reference bounding box
                                 0, 0, width, height,
                                 // Result bounding box
                                 width * x, height * y, width * (x + 1), height * (y + 1));

    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(JoinVideoEncoderTestFixture, test6x2) {
    const auto rows = 6u, columns = 2u;
    const auto height = 512u, width = 1024u;
    const auto inputFilename = "resources/test-pattern-1K.h264";

    EncodeConfiguration encodeConfiguration(rows*height, columns*width, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {rows*columns, decodeConfiguration1K}, encodeConfiguration, rows, columns);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};

    for(auto i = 0u; i < rows*columns; i++)
        readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 600);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);

    for(auto x = 0u; x < columns; x++)
        for(auto y = 0u; y < rows; y++)
            EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR,
                                 // Reference bounding box
                                 0, 0, width, height,
                                 // Result bounding box
                                 width * x, height * y, width * (x + 1), height * (y + 1));

    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(JoinVideoEncoderTestFixture, test2x8) {
    const auto rows = 2u, columns = 8u;
    const auto height = 512u, width = 1024u;
    const auto inputFilename = "resources/test-pattern-1K.h264";

    EncodeConfiguration encodeConfiguration(rows*height, columns*width, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {rows*columns, decodeConfiguration1K}, encodeConfiguration, rows, columns);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};

    for(auto i = 0u; i < rows*columns; i++)
        readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 600);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);

    for(auto x = 0u; x < columns; x++)
        for(auto y = 0u; y < rows; y++)
            EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR,
                                 // Reference bounding box
                                 0, 0, width, height,
                                 // Result bounding box
                                 width * x, height * y, width * (x + 1), height * (y + 1));

    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(JoinVideoEncoderTestFixture, test2x2_at_4K) {
    const auto rows = 2u, columns = 2u;
    const auto height = 2160u, width = 3840u;
    const auto inputFilename = "resources/test-pattern-4K.h264";

    EncodeConfiguration encodeConfiguration(rows*height, columns*width, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {rows*columns, decodeConfiguration4K}, encodeConfiguration, rows, columns);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};

    for(auto i = 0u; i < rows*columns; i++)
        readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 600);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);

    for(auto x = 0u; x < columns; x++)
        for(auto y = 0u; y < rows; y++)
        EXPECT_VIDEO_QUALITY(FILENAME(0), inputFilename, DEFAULT_PSNR,
                             // Reference bounding box
                             0, 0, width, height,
                             // Result bounding box
                             width * x, height * y, width * (x + 1), height * (y + 1));

    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(JoinVideoEncoderTestFixture, test2x2TileTransform) {
    const auto rows = 2u, columns = 2u;
    const auto height = 1080u, width = 1920u;
    const auto inputFilename = "resources/test-pattern.h264";
    const auto frames = 99u;

    auto transforms = 0u;
    EncodeConfiguration encodeConfiguration(rows*height, columns*width, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {rows*columns, decodeConfiguration2K}, encodeConfiguration, rows, columns);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};
    std::vector<FrameTransform> tileTransforms{4, [&transforms](VideoLock&, const Frame& frame) -> const Frame& {
        transforms++;
        return frame; }};

    for(auto i = 0u; i < rows*columns; i++)
        readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer, tileTransforms), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), frames);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);

    ASSERT_EQ(transforms, frames * rows * columns);
}

TEST_F(JoinVideoEncoderTestFixture, test2x2JoinedTransform) {
    const auto rows = 2u, columns = 2u;
    const auto height = 1080u, width = 1920u;
    const auto inputFilename = "resources/test-pattern.h264";
    const auto frames = 99u;

    auto transforms = 0u;
    EncodeConfiguration encodeConfiguration(rows*height, columns*width, NV_ENC_HEVC, 24, 24, 1024*1024);
    JoinVideoEncoder joiner(context, {rows*columns, decodeConfiguration2K}, encodeConfiguration, rows, columns);
    std::vector<std::shared_ptr<DecodeReader>> readers{};
    FileEncodeWriter writer{joiner.api(), FILENAME(0)};
    EncodableFrameTransform transform = [&transforms](VideoLock&, EncodeBuffer& frame) -> EncodeBuffer& {
        transforms++;
        return frame; };

    for(auto i = 0u; i < rows*columns; i++)
        readers.emplace_back(std::make_shared<FileDecodeReader>(inputFilename));

    ASSERT_EQ(joiner.join(readers, writer, transform), NV_ENC_SUCCESS);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), frames);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);

    ASSERT_EQ(transforms, frames);
}
