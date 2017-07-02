#include <gtest/gtest.h>
#include "EncodeAPI.h"
#include "VideoEncoder.h"

class EncodeBufferTestFixture : public testing::Test {
public:
    EncodeBufferTestFixture () :
            configuration("/dev/null", "/dev/null", 1080, 1920, 2, 2,
                          NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, 0),
            context(configuration.deviceID),
            encodeAPI(context)
    { }

protected:
    GPUContext context;
    EncodeAPI encodeAPI;
    EncodeConfig configuration;
};

TEST_F(EncodeBufferTestFixture, testBuffer) {
  EncodeBuffer encodeBuffer(encodeAPI, configuration);
}
