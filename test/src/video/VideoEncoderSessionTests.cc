#include "VideoEncoderSession.h"
#include "EncodeWriter.h"
#include <gtest/gtest.h>

#define FILENAME "resources/result-VideoEncoderSessionTestFixture.h265"

class VideoEncoderSessionTestFixture : public testing::Test {
public:
    VideoEncoderSessionTestFixture()
        : context(0),
          configuration(1080, 1920, 2, 2, NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, 0),
          lock(context),
          encoder(context, configuration, lock),
          writer(encoder, FILENAME),
          session(encoder, writer)
    {}

protected:
    GPUContext context;
    EncodeConfig configuration;
    VideoLock lock;
    VideoEncoder encoder;
    FileEncodeWriter writer;
    VideoEncoderSession session;
};


TEST_F(VideoEncoderSessionTestFixture, testConstructor) {
    EXPECT_EQ(session.frameCount(), 0);
}

TEST_F(VideoEncoderSessionTestFixture, testFlush) {
    ASSERT_EQ(session.Flush(), NV_ENC_SUCCESS);
    EXPECT_EQ(session.frameCount(), 0);
}

TEST_F(VideoEncoderSessionTestFixture, testEncodeSingleFrame) {
    CUdeviceptr handle;
    size_t pitch;

    ASSERT_EQ(cuMemAllocPitch(
        &handle,
        &pitch,
        configuration.width,
        configuration.height * (3/2) * 2,
        16), CUDA_SUCCESS);

    Frame inputFrame(handle, static_cast<unsigned int>(pitch), configuration, NV_ENC_PIC_STRUCT_FRAME);

    ASSERT_EQ(session.encode(inputFrame), NV_ENC_SUCCESS);

    EXPECT_EQ(session.frameCount(), 1);

    EXPECT_EQ(session.Flush(), NV_ENC_SUCCESS);
    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
    EXPECT_EQ(system("resources/assert-frames.sh " FILENAME " 1"), 0);

    EXPECT_EQ(remove(FILENAME), 0);
    EXPECT_EQ(cuMemFree(handle), CUDA_SUCCESS);
}

TEST_F(VideoEncoderSessionTestFixture, testEncodeMultipleFrames) {
    auto count = 60;
    CUdeviceptr handle;
    size_t pitch;

    ASSERT_EQ(cuMemAllocPitch(
            &handle,
            &pitch,
            configuration.width,
            configuration.height * (3/2) * 2,
            16), CUDA_SUCCESS);

    Frame inputFrame(handle, static_cast<unsigned int>(pitch), configuration, NV_ENC_PIC_STRUCT_FRAME);

    for(int i = 0; i < count; i++) {
        ASSERT_EQ(session.encode(inputFrame), NV_ENC_SUCCESS);
    }

    EXPECT_EQ(session.frameCount(), count);

    EXPECT_EQ(session.Flush(), NV_ENC_SUCCESS);
    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet " FILENAME), 0);
    EXPECT_EQ(system("resources/assert-frames.sh " FILENAME " 60"), 0);

    EXPECT_EQ(remove(FILENAME), 0);
    EXPECT_EQ(cuMemFree(handle), CUDA_SUCCESS);
}
