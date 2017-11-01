#ifndef CROPPER_H
#define CROPPER_H

#include "Transcoder.h"
#include <numeric>


class CropTranscoder: public Transcoder {
public:
    CropTranscoder(GPUContext& context,
               DecodeConfiguration &decodeConfiguration,
               EncodeConfiguration &encodeConfiguration)
            : Transcoder(context, decodeConfiguration, encodeConfiguration)
    { }

    void crop(DecodeReader &reader, EncodeWriter &writer, const size_t top, const size_t left) {
        crop(reader, writer, [](Frame& frame) -> Frame& { return frame; }, top, left);
    }

    void crop(DecodeReader &reader, EncodeWriter &writer, std::vector<FrameTransform> transforms,
              const size_t top, const size_t left) {
        crop(reader, writer, [transforms](Frame& frame) -> Frame& {
            return std::accumulate(
                transforms.begin(), transforms.end(),
                std::ref(frame),
                [](auto& frame, auto& f) -> Frame& { return f(frame); });
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
            auto processedFrame = transform(decodedFrame);

            for (auto i = 0u; i <= dropOrDuplicate; i++, framesEncoded++)
                encodeSession.Encode(processedFrame, top, left);
        }

        LOG(INFO) << "Crop complete (decoded " << framesDecoded << ", encoded " << framesEncoded << ")";
    }
};


#endif // CROPPER_H
