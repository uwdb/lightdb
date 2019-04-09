#include "EncodeAPI.h"
#include "VideoEncoder.h"
#include "EncodeBuffer.h"
#include <gtest/gtest.h>

class EncodeBufferTestFixture : public testing::Test {
public:
    EncodeBufferTestFixture () :
            configuration(Configuration{1920, 1080, 0, 0, 1024*1024, {30, 1}, {0, 0}}, NV_ENC_HEVC, 30),
            context(deviceId),
            lock(context),
            encoder(context, configuration, lock)
    { }

protected:
    const unsigned int deviceId = 0;
    EncodeConfiguration configuration;
    GPUContext context;
    VideoLock lock;
    VideoEncoder encoder;
};

TEST_F(EncodeBufferTestFixture, testBuffer) {
  ASSERT_NO_THROW(EncodeBuffer(encoder, 1024*1024));
}

TEST_F(EncodeBufferTestFixture, testLock) {
    EncodeBuffer encodeBuffer(encoder);

    {  std::scoped_lock{encodeBuffer}; }
    {  std::scoped_lock{encodeBuffer}; }
}