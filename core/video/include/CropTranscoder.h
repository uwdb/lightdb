#ifndef CROPPER_H
#define CROPPER_H

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "VideoEncoderSession.h"
#include "VideoDecoderSession.h"
#include "DecodeReader.h"
#include "EncodeWriter.h"
#include "FrameRateAlignment.h"
#include <numeric>


class CropTranscoder {
public:
    CropTranscoder(GPUContext& context,
               DecodeConfiguration &decodeConfiguration,
               EncodeConfiguration &encodeConfiguration)
            : lock_(context), frameQueue_(lock_.get()),
              encoder_(context, encodeConfiguration, lock_),
              decoder_(decodeConfiguration, frameQueue_, lock_)
    { }

    void crop(DecodeReader &reader, EncodeWriter &writer, const size_t top, const size_t left) {
        crop(reader, writer, [](VideoLock&, Frame& frame) -> Frame& { return frame; }, top, left);
    }

    void crop(DecodeReader &reader, EncodeWriter &writer, std::vector<FrameTransform> transforms,
              const size_t top, const size_t left) {
        crop(reader, writer, [this, transforms](VideoLock&, Frame& frame) -> Frame& {
            return std::accumulate(
                transforms.begin(), transforms.end(),
                std::ref(frame),
                [this](auto& frame, auto& f) -> Frame& { return f(lock_, frame); });
        }, top, left);
    }

    void crop(DecodeReader &reader, EncodeWriter &writer, FrameTransform transform,
              const size_t top, const size_t left) {
        VideoDecoderSession decodeSession(decoder_, reader);
        VideoEncoderSession encodeSession(encoder(), writer);
        FrameRateAlignment alignment(encoder().configuration().framerate, decoder().configuration().framerate);
        size_t framesDecoded = 0, framesEncoded = 0;

        while (!decoder().frame_queue().isComplete()) {
            auto dropOrDuplicate = alignment.dropOrDuplicate(framesDecoded++, framesEncoded);
            auto decodedFrame = decodeSession.decode();
            auto processedFrame = transform(lock_, decodedFrame);

            for (auto i = 0u; i <= dropOrDuplicate; i++, framesEncoded++)
                if(top == 0 && left == 0)
                    encodeSession.Encode(processedFrame);
                else
                    encodeSession.Encode(processedFrame, top, left);
        }

        LOG(INFO) << "Transcode complete (decoded " << framesDecoded << ", encoded " << framesEncoded << ")";
    }

    VideoEncoder &encoder() { return encoder_; }
    VideoDecoder &decoder() { return decoder_; }

private:
    VideoLock lock_;
    CUVIDFrameQueue frameQueue_;
    VideoEncoder encoder_;
    CudaDecoder decoder_;
};


#endif // CROPPER_H
