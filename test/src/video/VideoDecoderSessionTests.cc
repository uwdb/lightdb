#include "VideoEncoderSession.h"
#include "VideoDecoderSession.h"
#include "AssertVideo.h"
#include "TestResources.h"

class VideoDecoderSessionTestFixture : public testing::Test {
public:
    VideoDecoderSessionTestFixture()
        : context(0),
          configuration{Resources.videos.black.height, Resources.videos.black.width, 30, lightdb::Codec::h264()},
          lock(context),
          queue(lock),
          decoder(configuration, queue, lock),
          reader(Resources.videos.black.name)
    { }

protected:
    GPUContext context;
    DecodeConfiguration configuration;
    VideoLock lock;
    CUVIDFrameQueue queue;
    CudaDecoder decoder;
    FileDecodeReader reader;
};


TEST_F(VideoDecoderSessionTestFixture, testConstructor) {
    VideoDecoderSession session(decoder, reader.begin(), reader.end());
    ASSERT_EQ(session.decoder().handle(), decoder.handle());
}

TEST_F(VideoDecoderSessionTestFixture, testDecodeSingleFrame) {
    VideoDecoderSession<> session(decoder, reader.begin(), reader.end());

    auto frame = session.decode();
    ASSERT_EQ(frame.width(), configuration.width);
    ASSERT_EQ(frame.height(), configuration.height);
    ASSERT_BLACK_FRAME(frame);
}

TEST_F(VideoDecoderSessionTestFixture, testEncodeMultipleFrames) {
    auto count = 60;
    VideoDecoderSession<> session(decoder, reader.begin(), reader.end());

    for(int i = 0; i < count; i++) {
        auto frame = session.decode();
        ASSERT_BLACK_FRAME(frame);
    }
}