#include "VideoEncoder.h"
#include "VideoLock.h"
#include <gtest/gtest.h>

class VideoEncoderTestFixture : public testing::Test {
public:
    VideoEncoderTestFixture()
        : context(0),
          configuration(1080, 1920, NV_ENC_HEVC, 30, 30, 1024*1024),
          lock(context),
          encoder(context, configuration, lock)
    {}

protected:
    GPUContext context;
    EncodeConfiguration configuration;
    VideoLock lock;
    VideoEncoder encoder;
};


TEST_F(VideoEncoderTestFixture, testConstructor) {
    ASSERT_TRUE(encoder.api().encoderCreated());
    ASSERT_EQ(encoder.configuration().width, configuration.width);
}
