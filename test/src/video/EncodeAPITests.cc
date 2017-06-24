#include <gtest/gtest.h>
#include "EncodeAPI.h"
#include "VideoEncoder.h"

class EncodeAPITestFixture : public testing::Test {
public:
    EncodeAPITestFixture ()
    { }

    void SetUp() override {
      ASSERT_EQ(cuInit(0, __CUDA_API_VERSION, nullptr), CUDA_SUCCESS);
      ASSERT_EQ(cuDeviceGet(&device, 0), CUDA_SUCCESS);
      ASSERT_EQ(cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device), CUDA_SUCCESS);
    }

    void TearDown() override {
      ASSERT_EQ(cuCtxDestroy(context), CUDA_SUCCESS);
    }

protected:
    CUdevice device;
    CUcontext context;
    EncodeAPI encodeAPI;
    char* preset = "hq";
};

TEST_F(EncodeAPITestFixture, testConstructor) {
}

TEST_F(EncodeAPITestFixture, testInitialize) {
  ASSERT_EQ(encodeAPI.Initialize(context, NV_ENC_DEVICE_TYPE_CUDA), NV_ENC_SUCCESS);
  ASSERT_EQ(encodeAPI.Deinitialize(), NV_ENC_SUCCESS);
}

TEST_F(EncodeAPITestFixture, testPresetGUIDs) {
  ASSERT_EQ(encodeAPI.Initialize(context, NV_ENC_DEVICE_TYPE_CUDA), NV_ENC_SUCCESS);

  auto preset_guid = encodeAPI.GetPresetGUID(preset, NV_ENC_HEVC);

  ASSERT_EQ(preset_guid, NV_ENC_PRESET_HQ_GUID);

  ASSERT_EQ(encodeAPI.Deinitialize(), NV_ENC_SUCCESS);
}

TEST_F(EncodeAPITestFixture, testCreateEncoder) {
  EncodeConfig configuration("/dev/null", "/dev/null", 1080, 1920, 2, 2,
                             NV_ENC_HEVC, preset, 30, 30, 1024*1024, 0, 0);

  ASSERT_EQ(encodeAPI.Initialize(context, NV_ENC_DEVICE_TYPE_CUDA), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeAPI.CreateEncoder(&configuration), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeAPI.Deinitialize(), NV_ENC_SUCCESS);
}

TEST_F(EncodeAPITestFixture, testEncodeFrame) {
  EncodeConfig configuration("/dev/null", "/dev/null", 1080, 1920, 2, 2,
                             NV_ENC_HEVC, preset, 30, 30, 1024*1024, 0, 0);
  EncodeBuffer encodeBuffer(encodeAPI, configuration);

  ASSERT_EQ(encodeAPI.Initialize(context, NV_ENC_DEVICE_TYPE_CUDA), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeAPI.CreateEncoder(&configuration), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeBuffer.Initialize(), NV_ENC_SUCCESS);

  EXPECT_EQ(encodeAPI.NvEncEncodeFrame(&encodeBuffer, nullptr, NV_ENC_PIC_STRUCT_FRAME), NV_ENC_SUCCESS);

  EXPECT_EQ(encodeBuffer.Release(), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeAPI.Deinitialize(), NV_ENC_SUCCESS);
}

TEST_F(EncodeAPITestFixture, testProcessOutput) {
  EncodeConfig configuration("/dev/null", "/dev/null", 1080, 1920, 2, 2,
                             NV_ENC_HEVC, preset, 30, 30, 1024*1024, 0, 0);
  EncodeBuffer encodeBuffer(encodeAPI, configuration);

  configuration.fOutput = fopen(configuration.outputFileName, "wb");
  EXPECT_NE(configuration.fOutput, nullptr);

  ASSERT_EQ(encodeAPI.Initialize(context, NV_ENC_DEVICE_TYPE_CUDA), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeAPI.CreateEncoder(&configuration), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeBuffer.Initialize(), NV_ENC_SUCCESS);

  EXPECT_EQ(encodeAPI.NvEncEncodeFrame(&encodeBuffer, nullptr, NV_ENC_PIC_STRUCT_FRAME), NV_ENC_SUCCESS);

  //EXPECT_EQ(encodeAPI.ProcessOutput(&encodeBuffer), NV_ENC_SUCCESS);

  EXPECT_EQ(encodeBuffer.Release(), NV_ENC_SUCCESS);

  ASSERT_EQ(encodeAPI.Deinitialize(), NV_ENC_SUCCESS);

  EXPECT_EQ(fclose(configuration.fOutput), 0);
}
