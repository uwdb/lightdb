#include "Transcoder.h"
#include <gtest/gtest.h>

#define FILENAME "resources/test-pattern.h265"
#define FILENAME2 "resources/test-pattern-2.h265"

class TranscoderTestFixture : public testing::Test {
public:
    TranscoderTestFixture()
        : context(0),
          configuration(1080, 1920, 2, 2, NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, 0),
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
    ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                   FILENAME), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
    EXPECT_EQ(system("resources/assert-frames.sh " FILENAME " 99"), 0);
    EXPECT_EQ(remove(FILENAME), 0);
}

TEST_F(TranscoderTestFixture, testTranscoderWithWriter) {
    FileEncodeWriter writer(transcoder.encoder().api(), "resources/test-pattern.h265");

    ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264", writer), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
    EXPECT_EQ(system("resources/assert-frames.sh " FILENAME " 99"), 0);
    EXPECT_EQ(remove(FILENAME), 0);
}


TEST_F(TranscoderTestFixture, testTwoFileTranscoder) {
    ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                   FILENAME), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
    EXPECT_EQ(remove(FILENAME), 0);

    ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                   FILENAME2), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME2), 0);
    EXPECT_EQ(remove(FILENAME2), 0);
}

TEST_F(TranscoderTestFixture, testMultipleFileTranscoder) {
    for(int i = 0; i < 10; i++) {
        ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                       FILENAME), NV_ENC_SUCCESS);

        EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
        EXPECT_EQ(remove(FILENAME), 0);
    }
}
