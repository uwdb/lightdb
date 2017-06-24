#include <gtest/gtest.h>
#include "EncodeAPI.h"
#include "VideoEncoder.h"

class EncodeBufferTestFixture : public testing::Test {
public:
    EncodeBufferTestFixture () :
            configuration("/dev/null", "/dev/null", 1080, 1920, 2, 2,
                          NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, 0)
    { }

    void SetUp() override {
      ASSERT_EQ(cuInit(0, __CUDA_API_VERSION, nullptr), CUDA_SUCCESS);
      ASSERT_EQ(cuDeviceGet(&device, 0), CUDA_SUCCESS);
      ASSERT_EQ(cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device), CUDA_SUCCESS);
      ASSERT_EQ(encodeAPI.Initialize(context, NV_ENC_DEVICE_TYPE_CUDA), NV_ENC_SUCCESS);
      ASSERT_EQ(encodeAPI.CreateEncoder(&configuration), NV_ENC_SUCCESS);
    }

    void TearDown() override {
      ASSERT_EQ(encodeAPI.Deinitialize(), NV_ENC_SUCCESS);
      ASSERT_EQ(cuCtxDestroy(context), CUDA_SUCCESS);
    }

protected:
    CUdevice device;
    CUcontext context;
    EncodeAPI encodeAPI;
    EncodeConfig configuration;
};

TEST_F(EncodeBufferTestFixture, testBuffer) {
  EncodeBuffer encodeBuffer(encodeAPI, configuration);

  ASSERT_EQ(encodeBuffer.Initialize(), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeBuffer.Release(), NV_ENC_SUCCESS);
}
