#include "VideoEncoderSession.h"
#include "VideoDecoderSession.h"
#include "AssertVideo.h"
#include "TestResources.h"
#include "RequiresGPUTest.h"

class VideoDecoderSessionTestFixture : public RequiresGPUTest {
public:
    VideoDecoderSessionTestFixture()
        : configuration{Resources.videos.black.height, Resources.videos.black.width,
                        Resources.videos.black.mp4.fps, lightdb::Codec::h264()},
          queue([this]() { return CUVIDFrameQueue(lock); }),
          decoder([this]() { return CudaDecoder(configuration, queue, lock); }),
          reader([]() { return FileDecodeReader(Resources.videos.black.h264.name); })
    { }

protected:
    DecodeConfiguration configuration;
    lightdb::lazy<CUVIDFrameQueue> queue;
    lightdb::lazy<CudaDecoder> decoder;
    lightdb::lazy<FileDecodeReader> reader;
};


TEST_F(VideoDecoderSessionTestFixture, testConstructor) {
    VideoDecoderSession session(decoder, reader->begin(), reader->end());
    ASSERT_EQ(session.decoder().handle(), decoder->handle());
}

TEST_F(VideoDecoderSessionTestFixture, testDecodeSingleFrame) {
    VideoDecoderSession<> session(decoder, reader->begin(), reader->end());

    auto frame = session.decode();
    ASSERT_EQ(frame.width(), configuration.width);
    ASSERT_EQ(frame.height(), configuration.height);
    ASSERT_BLACK_FRAME(frame);
}

TEST_F(VideoDecoderSessionTestFixture, testDecodeMultipleFrames) {
    auto count = 60;
    VideoDecoderSession<> session(decoder, reader->begin(), reader->end());

    for(int i = 0; i < count; i++) {
        auto frame = session.decode();
        ASSERT_BLACK_FRAME(frame);
    }
}