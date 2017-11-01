#include "CropTranscoder.h"
#include "AssertTime.h"
#include "AssertVideo.h"

#define FILENAME(n) (std::string("resources/test-pattern-") + std::to_string(n) + ".h265")

class CropperTestFixture : public testing::Test {
public:
    CropperTestFixture()
        : context(0),
          decodeConfiguration{1920, 1080, 24, cudaVideoCodec_H264}
    {}

protected:
    GPUContext context;
    DecodeConfiguration decodeConfiguration;
};


TEST_F(CropperTestFixture, testConstructor) {
}

TEST_F(CropperTestFixture, testFileCropper) {
    auto top = 200u, left = 200u, width = 512u, height = 128u;
    EncodeConfiguration encodeConfiguration{height, width, NV_ENC_HEVC, 24, 24, 1024*1024};
    CropTranscoder cropper(context, decodeConfiguration, encodeConfiguration);
    FileDecodeReader reader("resources/test-pattern.h264");
    FileEncodeWriter writer(cropper.encoder().api(), FILENAME(0));

    ASSERT_SECS(
            ASSERT_NO_THROW(cropper.crop(reader, writer, top, left)),
        1.5);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}
