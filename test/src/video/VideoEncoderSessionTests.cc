#include "VideoEncoderSession.h"
#include "TestResources.h"
#include "AssertVideo.h"
#include <gtest/gtest.h>

#define FILENAME "resources/result-VideoEncoderSessionTestFixture.h265"

class VideoEncoderSessionTestFixture : public testing::Test {
public:
    VideoEncoderSessionTestFixture()
        : context(0),
          configuration({1920, 1080, 0, 0, 1024*1024, {24, 1}}, NV_ENC_HEVC, 30),
          lock(context),
          encoder(context, configuration, lock),
          writer(encoder, FILENAME),
          session(encoder, writer)
    {}

protected:
    GPUContext context;
    EncodeConfiguration configuration;
    VideoLock lock;
    VideoEncoder encoder;
    FileEncodeWriter writer;
    VideoEncoderSession session;
};


TEST_F(VideoEncoderSessionTestFixture, testConstructor) {
    EXPECT_EQ(session.frameCount(), 0);
}

TEST_F(VideoEncoderSessionTestFixture, testFlush) {
    ASSERT_NO_THROW(session.Flush());
    EXPECT_EQ(session.frameCount(), 0);
}

TEST_F(VideoEncoderSessionTestFixture, testEncodeSingleFrame) {
    auto blackFrame = CREATE_BLACK_FRAME(configuration);

    ASSERT_NO_THROW(session.Encode(blackFrame));

    EXPECT_EQ(session.frameCount(), 1);

    EXPECT_NO_THROW(session.Flush());

    EXPECT_VIDEO_VALID(FILENAME);
    EXPECT_VIDEO_FRAMES(FILENAME, 1);
    EXPECT_VIDEO_RESOLUTION(FILENAME, configuration.height, configuration.width);
    EXPECT_VIDEO_QUALITY(FILENAME, Resources.videos.black.h264.name, DEFAULT_PSNR, 0, 0,
                                   configuration.width, configuration.height);

    EXPECT_EQ(remove(FILENAME), 0);
    EXPECT_EQ(cuMemFree(blackFrame.handle()), CUDA_SUCCESS);
}

TEST_F(VideoEncoderSessionTestFixture, testEncodeMultipleFrames) {
    auto count = 60;
    auto blackFrame = CREATE_BLACK_FRAME(configuration);

    for(int i = 0; i < count; i++)
        ASSERT_NO_THROW(session.Encode(blackFrame));

    EXPECT_EQ(session.frameCount(), count);

    EXPECT_NO_THROW(session.Flush());
    EXPECT_VIDEO_VALID(FILENAME);
    EXPECT_VIDEO_FRAMES(FILENAME, 60);
    EXPECT_VIDEO_RESOLUTION(FILENAME, configuration.height, configuration.width);

    EXPECT_EQ(remove(FILENAME), 0);
    EXPECT_EQ(cuMemFree(blackFrame.handle()), CUDA_SUCCESS);
}
