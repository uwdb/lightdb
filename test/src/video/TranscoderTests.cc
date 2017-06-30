#include "Transcoder.h"
#include <gtest/gtest.h>

class TranscoderTestFixture : public testing::Test {
public:
    TranscoderTestFixture()
        : context(0),
          configuration("/dev/null", "/dev/null", 1080, 1920, 2, 2, NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, 0),
          transcoder(context, configuration)
    {}

    void SetUp() override {
        //ASSERT_EQ(cuInit(0, __CUDA_API_VERSION, nullptr), CUDA_SUCCESS);
        //ASSERT_EQ(cuvidInit(0), CUDA_SUCCESS);
        //ASSERT_EQ(cuDeviceGet(&device, 0), CUDA_SUCCESS);
        //ASSERT_EQ(cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device), CUDA_SUCCESS);
        //ASSERT_NE((transcoder = new Transcoder(context.get(), configuration)), nullptr);
    }

    void TearDown() override {
        //delete transcoder;
        //ASSERT_EQ(cuCtxDestroy(context), CUDA_SUCCESS);
    }

protected:
    GPUContext context;

    //CUdevice device;
    //CUcontext context = nullptr;
    EncodeConfig configuration;
    Transcoder transcoder;

};

TEST_F(TranscoderTestFixture, testConstructor) {
}

TEST_F(TranscoderTestFixture, testFileTranscoder) {
    ASSERT_EQ(transcoder.transcode("resources/test-pattern.h264",
                                   "resources/test-pattern.h265"), NV_ENC_SUCCESS);

    EXPECT_EQ(system("ffprobe -hide_banner -loglevel quiet resources/test-pattern.h265"), 0);
    ASSERT_EQ(remove("resources/test-pattern.h265"), 0);
}
