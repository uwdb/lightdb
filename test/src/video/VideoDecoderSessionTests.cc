#include "VideoEncoderSession.h"
#include "AssertVideo.h"
#include <VideoDecoderSession.h>

#define FILENAME "resources/black.h264"

class VideoDecoderSessionTestFixture : public testing::Test {
public:
    VideoDecoderSessionTestFixture()
        //: context(0),
        //  configuration{3840, 2160, 30, lightdb::Codec::h264()},
        //  lock(context),
        //  queue(lock),
        //  decoder(configuration, queue, lock),
        //  reader(FILENAME)
    {}

protected:
//    GPUContext context;
//    DecodeConfiguration configuration;
//    VideoLock lock;
//    CUVIDFrameQueue queue;
//    CudaDecoder decoder;
//    FileDecodeReader reader;
};


TEST_F(VideoDecoderSessionTestFixture, testConstructor) {
    //VideoDecoderSession session(decoder, reader.begin(), reader.end());
    //ASSERT_EQ(session.decoder().handle(), decoder.handle());
    FAIL();
}

TEST_F(VideoDecoderSessionTestFixture, testDecodeSingleFrame) {
/*    VideoDecoderSession<> session(decoder, reader.begin(), reader.end());

    auto frame = session.decode();
    ASSERT_EQ(frame.width(), configuration.width);
    ASSERT_EQ(frame.height(), configuration.height);
    ASSERT_BLACK_FRAME(frame);*/
    FAIL();
}

TEST_F(VideoDecoderSessionTestFixture, testEncodeMultipleFrames) {
    /*auto count = 60;
    VideoDecoderSession<> session(decoder, reader.begin(), reader.end());

    for(int i = 0; i < count; i++) {
        auto frame = session.decode();
        ASSERT_BLACK_FRAME(frame);
    }*/
    FAIL();
}