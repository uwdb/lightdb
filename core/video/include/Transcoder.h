#ifndef TRANSCODER_H
#define TRANSCODER_H

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "VideoEncoderSession.h"
#include "VideoDecoderSession.h"
#include "DecodeReader.h"
#include "EncodeWriter.h"
#include "FrameRateAlignment.h"
#include <numeric>


class Transcoder {
public:
    Transcoder(GPUContext& context,
               DecodeConfiguration &decodeConfiguration,
               EncodeConfiguration &encodeConfiguration)
            : lock_(context), frameQueue_(lock_.get()),
              encoder_(context, encodeConfiguration, lock_),
              decoder_(decodeConfiguration, frameQueue_, lock_)
    { }

    void transcode(DecodeReader &reader, EncodeWriter &writer) {
        transcode(reader, writer, [](Frame& frame) -> Frame& { return frame; });
    }

    void transcode(DecodeReader &reader, EncodeWriter &writer, std::vector<FrameTransform> transforms) {
        transcode(reader, writer, [transforms](Frame& frame) -> Frame& {
            return std::accumulate(
                    transforms.begin(), transforms.end(),
                    std::ref(frame),
                    [](auto& frame, auto& f) -> Frame& { return f(frame); });
        });
    }

    void transcode(DecodeReader &reader, EncodeWriter &writer, FrameTransform transform) {
        VideoDecoderSession decodeSession(decoder_, reader);
        VideoEncoderSession encodeSession(encoder_, writer);
        FrameRateAlignment alignment(encoder_.configuration().framerate, decoder_.configuration().framerate);
        size_t framesDecoded = 0, framesEncoded = 0;

        while (!decoder_.frame_queue().isComplete()) {
            auto dropOrDuplicate = alignment.dropOrDuplicate(framesDecoded++, framesEncoded);
            auto decodedFrame = decodeSession.decode();
            auto processedFrame = transform(decodedFrame);

            for (auto i = 0u; i <= dropOrDuplicate; i++, framesEncoded++)
                encodeSession.Encode(processedFrame);
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


#endif
