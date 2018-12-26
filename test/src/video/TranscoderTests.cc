#include "AssertTime.h"
#include "AssertVideo.h"
#include "Transcoder.h"

#define FILENAME(n) (std::string("resources/test-pattern-") + std::to_string(n) + ".h265")

class TranscoderTestFixture : public testing::Test {
public:
    TranscoderTestFixture()
        : context(0),
          encodeConfiguration({1920, 1080, 0, 0, 1024*1024, {24, 1}}, NV_ENC_HEVC, 30),
          decodeConfiguration(encodeConfiguration, lightdb::Codec::h264()),
          transcoder(context, decodeConfiguration, encodeConfiguration)
    {}

protected:
    GPUContext context;
    EncodeConfiguration encodeConfiguration;
    DecodeConfiguration decodeConfiguration;
    Transcoder transcoder;

};


TEST_F(TranscoderTestFixture, testConstructor) {
}

TEST_F(TranscoderTestFixture, testFileTranscoder) {
    FileDecodeReader reader("resources/test-pattern.h264");
    FileEncodeWriter writer(transcoder.encoder().api(), FILENAME(0));

    ASSERT_SECS(
        ASSERT_NO_THROW(transcoder.transcode(reader, writer)),
        1.5);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_VIDEO_QUALITY(FILENAME(0), reader.filename(), DEFAULT_PSNR);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(TranscoderTestFixture, testTwoFileTranscoder) {
    FileDecodeReader reader1("resources/test-pattern.h264");
    FileEncodeWriter writer1(transcoder.encoder().api(), FILENAME(0));
    FileDecodeReader reader2("resources/test-pattern.h264");
    FileEncodeWriter writer2(transcoder.encoder().api(), FILENAME(1));

    ASSERT_SECS(
        ASSERT_NO_THROW(transcoder.transcode(reader1, writer1)),
        1.5);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_VIDEO_QUALITY(FILENAME(0), reader1.filename(), DEFAULT_PSNR);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);

    ASSERT_SECS(
        ASSERT_NO_THROW(transcoder.transcode(reader2, writer2)),
        1.5);

    EXPECT_VIDEO_VALID(FILENAME(1));
    EXPECT_VIDEO_FRAMES(FILENAME(1), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(1), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_VIDEO_QUALITY(FILENAME(1), reader2.filename(), DEFAULT_PSNR);
    EXPECT_EQ(remove(FILENAME(1).c_str()), 0);
}

TEST_F(TranscoderTestFixture, testMultipleFileTranscoder) {
    ASSERT_SECS(
        for(auto i = 0; i < 10; i++) {
            FileDecodeReader reader("resources/test-pattern.h264");
            FileEncodeWriter writer(transcoder.encoder().api(), FILENAME(i));

            EXPECT_NO_THROW(transcoder.transcode(reader, writer));
        }, 8);

    for(int i = 0; i < 10; i++) {
        EXPECT_VIDEO_VALID(FILENAME(i));
        EXPECT_VIDEO_FRAMES(FILENAME(i), 99);
        EXPECT_VIDEO_RESOLUTION(FILENAME(i), encodeConfiguration.height, encodeConfiguration.width);
        EXPECT_VIDEO_QUALITY(FILENAME(i), "resources/test-pattern.h264", DEFAULT_PSNR);
        EXPECT_EQ(remove(FILENAME(i).c_str()), 0);
    }
}

TEST_F(TranscoderTestFixture, testTranscoderWithIdentityTransform) {
    FileDecodeReader reader("resources/test-pattern.h264");
    FileEncodeWriter writer(transcoder.encoder().api(), FILENAME(0));
    FrameTransform identityTransform = [](VideoLock&, Frame& frame) -> Frame& { return frame; };

    ASSERT_SECS(
            ASSERT_NO_THROW(transcoder.transcode(reader, writer, identityTransform)),
            1.5);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_VIDEO_QUALITY(FILENAME(0), reader.filename(), DEFAULT_PSNR);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(TranscoderTestFixture, testTranscoderWithComplexTransform) {
    FileDecodeReader reader("resources/test-pattern.h264");
    FileEncodeWriter writer(transcoder.encoder().api(), FILENAME(0));
    auto height = reader.format().display_area.bottom, width = reader.format().display_area.right;

    FrameTransform halfBlackTransform = [](VideoLock& lock, Frame& frame) -> Frame& {
        CUresult result;
        std::scoped_lock mutex{lock};

        auto cudaFrame = dynamic_cast<GPUFrame&>(frame).cuda();
        if((result = cuMemsetD2D8(cudaFrame->handle(), cudaFrame->pitch(), 0,
                                  cudaFrame->width() / 2, cudaFrame->height())) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuMemsetD2D8 failed.", result);
        return frame;
    };

    ASSERT_SECS(
            ASSERT_NO_THROW(transcoder.transcode(reader, writer, halfBlackTransform)),
            1.75);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    // Right half original video
    EXPECT_VIDEO_QUALITY(FILENAME(0), reader.filename(), DEFAULT_PSNR, width/2, 0, width, height,
                                                                       width/2, 0, width, height);
    // Left half black
    EXPECT_VIDEO_QUALITY(FILENAME(0), "resources/black.h264", DEFAULT_PSNR, 0, 0, width/2, height,
                                                                            0, 0, width/2, height);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(TranscoderTestFixture, testTranscoderWithMultipleTransform) {
    FileDecodeReader reader("resources/test-pattern.h264");
    FileEncodeWriter writer(transcoder.encoder().api(), FILENAME(0));
    auto height = reader.format().display_area.bottom, width = reader.format().display_area.right;

    FrameTransform leftHalfBlackTransform = [](VideoLock& lock, Frame& frame) -> Frame& {
        std::scoped_lock mutex{lock};

        auto cudaFrame = dynamic_cast<GPUFrame&>(frame).cuda();
        assert(cuMemsetD2D8(cudaFrame->handle(), cudaFrame->pitch(), 0,
                            cudaFrame->width() / 2, cudaFrame->height()) == CUDA_SUCCESS);
        return frame;
    };
    FrameTransform topHalfBlackTransform = [](VideoLock& lock, Frame& frame) -> Frame& {
        std::scoped_lock mutex{lock};

        auto cudaFrame = dynamic_cast<GPUFrame&>(frame).cuda();
        assert(cuMemsetD2D8(cudaFrame->handle(), cudaFrame->pitch(), 0,
                            cudaFrame->width(), cudaFrame->height() / 2) == CUDA_SUCCESS);
        return frame;
    };

    ASSERT_SECS(
            ASSERT_NO_THROW(transcoder.transcode(reader, writer, {leftHalfBlackTransform, topHalfBlackTransform})),
            1.5);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 99);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    // Left half black
    EXPECT_VIDEO_QUALITY(FILENAME(0), "resources/black.h264", DEFAULT_PSNR, 0, 0, width/2, height,
                                                                            0, 0, width/2, height);
    // Top half black
    EXPECT_VIDEO_QUALITY(FILENAME(0), "resources/black.h264", DEFAULT_PSNR, 0, 0, width, height/2,
                                                                            0, 0, width, height/2);
    // Bottom-right corner original video
    EXPECT_VIDEO_QUALITY(FILENAME(0), reader.filename(), DEFAULT_PSNR, width/2, height/2, width, height,
                                                                       width/2, height/2, width, height);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}

TEST_F(TranscoderTestFixture, testTranscoderAt4K) {
    EncodeConfiguration encodeConfiguration({3840, 2160, 0, 0, 4*1024*1024, {30, 1}}, NV_ENC_HEVC, 30);
    DecodeConfiguration decodeConfiguration(encodeConfiguration, lightdb::Codec::h264());
    FileDecodeReader reader("resources/test-pattern-4K.h264");
    FileEncodeWriter writer(transcoder.encoder().api(), FILENAME(0));
    Transcoder transcoder(context, decodeConfiguration, encodeConfiguration);

    ASSERT_SECS(
            ASSERT_NO_THROW(transcoder.transcode(reader, writer)),
            25);

    EXPECT_VIDEO_VALID(FILENAME(0));
    EXPECT_VIDEO_FRAMES(FILENAME(0), 600);
    EXPECT_VIDEO_RESOLUTION(FILENAME(0), encodeConfiguration.height, encodeConfiguration.width);
    EXPECT_VIDEO_QUALITY(FILENAME(0), reader.filename(), DEFAULT_PSNR);
    EXPECT_EQ(remove(FILENAME(0).c_str()), 0);
}
