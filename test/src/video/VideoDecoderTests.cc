#include "VideoEncoder.h"
#include "VideoLock.h"
#include <gtest/gtest.h>
#include <FrameQueue.h>
#include <VideoDecoder.h>

class VideoDecoderTestFixture : public testing::Test {
public:
    VideoDecoderTestFixture()
        : context(0),
          configuration{1920, 1080, 30, cudaVideoCodec_H264},
          lock(context),
          queue(lock)
    {}

protected:
    GPUContext context;
    DecodeConfiguration configuration;
    VideoLock lock;
    CUVIDFrameQueue queue;
};


TEST_F(VideoDecoderTestFixture, testCudaConstructor) {
    CudaDecoder decoder(configuration, queue, lock);
    ASSERT_EQ(decoder.configuration().width, configuration.width);
    ASSERT_EQ(decoder.configuration().height, configuration.height);
    ASSERT_EQ(&decoder.frame_queue(), &queue);
}
