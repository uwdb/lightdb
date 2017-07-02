#include "Transcoder.h"
#include <gtest/gtest.h>

class TranscoderTestFixture : public testing::Test {
public:
    TranscoderTestFixture()
        : context(0),
          api(context.get()),
          configuration("/dev/null", "/dev/null", 1080, 1920, 2, 2, NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, 0),
          transcoder(context, api, configuration)
    {}

protected:
    GPUContext context;
    EncodeAPI api;
    EncodeConfig configuration;
    Transcoder transcoder;

};

TEST_F(TranscoderTestFixture, testConstructor) {
}

TEST_F(TranscoderTestFixture, testFileTranscoder) {
    ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                   "resources/test-pattern.h265"), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet resources/test-pattern.h265"), 0);
    EXPECT_EQ(remove("resources/test-pattern.h265"), 0);
}

TEST_F(TranscoderTestFixture, testTwoFileTranscoder) {
    ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                   "resources/test-pattern-1.h265"), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet resources/test-pattern-1.h265"), 0);
    EXPECT_EQ(remove("resources/test-pattern-1.h265"), 0);

    ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                   "resources/test-pattern-2.h265"), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet resources/test-pattern-2.h265"), 0);
    EXPECT_EQ(remove("resources/test-pattern-2.h265"), 0);
}

/*
TEST_F(TranscoderTestFixture, testMultipleFileTranscoder) {
    for(int i = 0; i < 10; i++) {
        ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                       "resources/test-pattern.h265"), NV_ENC_SUCCESS);

        EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet resources/test-pattern.h265"), 0);
        EXPECT_EQ(remove("resources/test-pattern.h265"), 0);
    }
}*/
