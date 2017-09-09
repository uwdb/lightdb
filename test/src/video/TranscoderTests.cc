#include "Transcoder.h"
#include "AssertTime.h"
#include "AssertVideo.h"

#define FILENAME(n) (std::string("resources/test-pattern-") + std::to_string(n) + ".h265")

class TranscoderTestFixture : public testing::Test {
public:
    TranscoderTestFixture()
        : context(0),
          configuration(1088, 1920, 2, 2, NV_ENC_HEVC, "hq", 24, 30, 1024*1024, 0, 0),
          transcoder(context, configuration)
    {}

protected:
    GPUContext context;
    EncodeConfig configuration;
    Transcoder transcoder;

};


TEST_F(TranscoderTestFixture, testConstructor) {
}

TEST_F(TranscoderTestFixture, testFileTranscoder) {
    FileDecodeReader reader("resources/test-pattern.h264");
    FileEncodeWriter writer(transcoder.encoder().api(), FILENAME(0));

    ASSERT_SECS(
        ASSERT_EQ(transcoder.transcode(reader, writer), NV_ENC_SUCCESS),
        1.5);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), configuration.height, configuration.width);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(TranscoderTestFixture, testTwoFileTranscoder) {
    FileDecodeReader reader1("resources/test-pattern.h264");
    FileEncodeWriter writer1(transcoder.encoder().api(), FILENAME(0));
    FileDecodeReader reader2("resources/test-pattern.h264");
    FileEncodeWriter writer2(transcoder.encoder().api(), FILENAME(1));

    ASSERT_SECS(
        ASSERT_EQ(transcoder.transcode(reader1, writer1), NV_ENC_SUCCESS),
        1.5);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), configuration.height, configuration.width);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);

    ASSERT_SECS(
        ASSERT_EQ(transcoder.transcode(reader2, writer2), NV_ENC_SUCCESS),
        1.5);

    EXPECT_VIDEO_VALID(FILENAME(1));
    EXPECT_VIDEO_FRAMES(FILENAME(1), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(1), configuration.height, configuration.width);
    EXPECT_EQ(remove(FILENAME(1).c_str()), 0);
}

TEST_F(TranscoderTestFixture, testMultipleFileTranscoder) {
    ASSERT_SECS(
        for(auto i = 0; i < 10; i++) {
            FileDecodeReader reader("resources/test-pattern.h264");
            FileEncodeWriter writer(transcoder.encoder().api(), FILENAME(i));

            EXPECT_EQ(transcoder.transcode(reader, writer), NV_ENC_SUCCESS);
        }, 3.5);

    for(int i = 0; i < 10; i++) {
        EXPECT_VIDEO_VALID(FILENAME(i));
        EXPECT_VIDEO_FRAMES(FILENAME(i), 99);
        EXPECT_VIDEO_RESOLUTION(FILENAME(i), configuration.height, configuration.width);
        EXPECT_EQ(remove(FILENAME(i).c_str()), 0);
    }
}
