#include "EncodeAPI.h"
#include "VideoEncoder.h"
#include <gtest/gtest.h>

class EncodeBufferTestFixture : public testing::Test {
public:
    EncodeBufferTestFixture () :
            configuration(1080, 1920, 2, 2, NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, deviceId),
            context(deviceId),
            encodeAPI(context)
    { }

protected:
    const unsigned int deviceId = 0;
    EncodeConfig configuration;
    GPUContext context;
    EncodeAPI encodeAPI;
};

TEST_F(EncodeBufferTestFixture, testBuffer) {
  ASSERT_EQ(configuration.deviceID, deviceId);

  EncodeBuffer encodeBuffer(encodeAPI, configuration);
}
