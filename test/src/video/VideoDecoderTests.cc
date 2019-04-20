#include "VideoEncoder.h"
#include "VideoLock.h"
#include "FrameQueue.h"
#include "VideoDecoder.h"
#include "RequiresGPUTest.h"

class VideoDecoderTestFixture : public RequiresGPUTest {
public:
    VideoDecoderTestFixture()
        : configuration{1920, 1080, 30, lightdb::Codec::h264()},
          queue([this]() { return CUVIDFrameQueue(lock); })
    {}

protected:
    DecodeConfiguration configuration;
    lightdb::lazy<CUVIDFrameQueue> queue;
};


TEST_F(VideoDecoderTestFixture, testCudaConstructor) {
    CudaDecoder decoder(configuration, queue, lock);
    ASSERT_EQ(decoder.configuration().width, configuration.width);
    ASSERT_EQ(decoder.configuration().height, configuration.height);
    ASSERT_EQ(&decoder.frame_queue(), &queue.value());
}
