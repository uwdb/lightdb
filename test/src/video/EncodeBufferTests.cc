#include "EncodeAPI.h"
#include "VideoEncoder.h"
#include "EncodeBuffer.h"
#include "RequiresGPUTest.h"

using lightdb::lazy;

class EncodeBufferTestFixture : public RequiresGPUTest {
public:
    EncodeBufferTestFixture ()
        : configuration(Configuration{1920, 1080, 0, 0, 1024*1024, {30, 1}, {0, 0}}, NV_ENC_HEVC, 30),
          encoder([this]{ return VideoEncoder(context, configuration, lock); })
    { }

protected:
    EncodeConfiguration configuration;
    lazy<VideoEncoder> encoder;
};

TEST_F(EncodeBufferTestFixture, testBuffer) {
  ASSERT_NO_THROW(EncodeBuffer(encoder, 1024*1024));
}

TEST_F(EncodeBufferTestFixture, testLock) {
    EncodeBuffer encodeBuffer(encoder);

    {  std::scoped_lock{encodeBuffer}; }
    {  std::scoped_lock{encodeBuffer}; }
}