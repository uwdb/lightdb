#include "Transcoder.h"
#include <gtest/gtest.h>

#define FILENAME "resources/test-pattern.h265"
#define FILENAME2 "resources/test-pattern-2.h265"

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
    FileEncodeWriter writer(transcoder.encoder().api(), FILENAME);

    ASSERT_EQ(transcoder.transcode(reader, writer), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
    EXPECT_EQ(system("resources/assert-frames.sh " FILENAME " 99"), 0);
    EXPECT_EQ(remove(FILENAME), 0);
}

TEST_F(TranscoderTestFixture, testTwoFileTranscoder) {
    FileDecodeReader reader1("resources/test-pattern.h264");
    FileEncodeWriter writer1(transcoder.encoder().api(), FILENAME);
    FileDecodeReader reader2("resources/test-pattern.h264");
    FileEncodeWriter writer2(transcoder.encoder().api(), FILENAME2);

    ASSERT_EQ(transcoder.transcode(reader1, writer1), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
    EXPECT_EQ(remove(FILENAME), 0);

    ASSERT_EQ(transcoder.transcode(reader2, writer2), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME2), 0);
    EXPECT_EQ(remove(FILENAME2), 0);
}

TEST_F(TranscoderTestFixture, testMultipleFileTranscoder) {
    for(int i = 0; i < 10; i++) {
        FileDecodeReader reader("resources/test-pattern.h264");
        FileEncodeWriter writer(transcoder.encoder().api(), FILENAME);

        ASSERT_EQ(transcoder.transcode(reader, writer), NV_ENC_SUCCESS);

        EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
        EXPECT_EQ(remove(FILENAME), 0);
    }
}
