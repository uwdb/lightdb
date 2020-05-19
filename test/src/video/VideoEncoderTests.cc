#include "VideoEncoder.h"
#include "VideoLock.h"
#include "RequiresGPUTest.h"

class VideoEncoderTestFixture : public RequiresGPUTest {
public:
    VideoEncoderTestFixture()
        : configuration({1920, 1080, 0, 0, 1024*1024, {30, 1}, {0, 0}}, NV_ENC_HEVC, 30),
          encoder([this]() { return VideoEncoder(context, configuration, lock); })
    {}

protected:
    EncodeConfiguration configuration;
    lightdb::lazy<VideoEncoder> encoder;
};


TEST_F(VideoEncoderTestFixture, testConstructor) {
    ASSERT_TRUE(encoder->api().encoderCreated());
    ASSERT_EQ(encoder->configuration().width, configuration.width);
}
