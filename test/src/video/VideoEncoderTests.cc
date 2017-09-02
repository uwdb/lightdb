#include "VideoEncoder.h"
#include "VideoLock.h"
#include <gtest/gtest.h>

class VideoEncoderTestFixture : public testing::Test {
public:
    VideoEncoderTestFixture()
        : context(0),
          configuration(1080, 1920, 2, 2, NV_ENC_HEVC, "hq", 30, 30, 1024*1024, 0, 0),
          lock(context),
          encoder(context, configuration, lock)
    {}

protected:
    GPUContext context;
    EncodeConfig configuration;
    VideoLock lock;
    VideoEncoder encoder;
};


TEST_F(VideoEncoderTestFixture, testConstructor) {
    ASSERT_TRUE(encoder.api().encoderCreated());
    ASSERT_EQ(encoder.configuration().width, configuration.width);
}
