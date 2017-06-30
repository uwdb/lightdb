#include "Transcoder.h"
#include <gtest/gtest.h>

class TranscoderTestFixture : public testing::Test {
public:
  TranscoderTestFixture()
          : transcoder(1080, 1920, NV_ENC_HEVC, "hq", 30, 30, 1024, 0, 0)
  {}

    void SetUp() override {
  }

  void TearDown() override {
  }

protected:
    Transcoder transcoder;

};
/*
TEST_F(TranscoderTestFixture, testConstructor) {
}

TEST_F(TranscoderTestFixture, testInitialize) {
  ASSERT_EQ(transcoder.initialize(), NV_ENC_SUCCESS);
}

TEST_F(TranscoderTestFixture, testFileTranscoder) {
  ASSERT_EQ(transcoder.initialize(), NV_ENC_SUCCESS);
  ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                 "resources/test-pattern.h265"), NV_ENC_SUCCESS);

  EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet resources/test-pattern.h265"), 0);
  ASSERT_EQ(remove("resources/test-pattern.h265"), 0);
}*/

//TEST_F(TranscoderTestFixture, testFileTranscoderReuse) { }


class Transcoder2TestFixture : public testing::Test {
public:
    Transcoder2TestFixture()
        : configuration("/dev/null", "/dev/null", 1080, 1920, 2, 2, NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, 0)
    {}

    void SetUp() override {
        ASSERT_EQ(cuInit(0, __CUDA_API_VERSION, nullptr), CUDA_SUCCESS);
        ASSERT_EQ(cuvidInit(0), CUDA_SUCCESS);
        ASSERT_EQ(cuDeviceGet(&device, 0), CUDA_SUCCESS);
        ASSERT_EQ(cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device), CUDA_SUCCESS);
        ASSERT_NE((transcoder = new Transcoder2(context, configuration)), nullptr);
    }

    void TearDown() override {
        delete transcoder;
        ASSERT_EQ(cuCtxDestroy(context), CUDA_SUCCESS);
    }

protected:
    CUdevice device;
    CUcontext context = nullptr;
    EncodeConfig configuration;
    Transcoder2* transcoder;

};

TEST_F(Transcoder2TestFixture, testConstructor) {
}

TEST_F(Transcoder2TestFixture, testFileTranscoder2) {
    ASSERT_EQ(transcoder->transcode("resources/test-pattern.h264",
                                    "resources/test-pattern.h265"), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet resources/test-pattern.h265"), 0);
    ASSERT_EQ(remove("resources/test-pattern.h265"), 0);
}
