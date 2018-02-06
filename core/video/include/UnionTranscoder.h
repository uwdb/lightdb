#ifndef LIGHTDB_UNIONTRANSCODER_H
#define LIGHTDB_UNIONTRANSCODER_H

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "VideoEncoderSession.h"
#include "VideoDecoderSession.h"
#include "DecodeReader.h"
#include "EncodeWriter.h"
#include "FrameRateAlignment.h"
#include <numeric>


class UnionTranscoder {
public:
    UnionTranscoder(GPUContext& context,
               std::vector<DecodeConfiguration> &decodeConfigurations,
               EncodeConfiguration &encodeConfiguration)
            : lock_(context), frameQueue_(lock_.get()),
              encoder_(context, encodeConfiguration, lock_)
    {
        std::for_each(decodeConfigurations.begin(), decodeConfigurations.end(), [this](auto &c) {
            decoders_.emplace_back(std::make_shared<CudaDecoder>(c, frameQueue_, lock_)); });
    }

    //TODO DecodeReaders should be shared_refs or something, not raw pointers
    void Union(std::vector<DecodeReader*> &readers, EncodeWriter &writer, const NaryFrameTransform &transform,
               const std::optional<size_t> frames={}) {
        std::vector<std::shared_ptr<VideoDecoderSession>> decodeSessions;
        VideoEncoderSession encodeSession(encoder(), writer);
        FrameRateAlignment alignment(encoder().configuration().framerate, decoders()[0]->configuration().framerate);
        size_t framesDecoded = 0, framesEncoded = 0, remaining = frames.value_or(std::numeric_limits<size_t>::max());

        for(auto i = 0u; i < readers.size(); i++)
            decodeSessions.emplace_back(std::make_shared<VideoDecoderSession>(*decoders_[i], *readers[i]));

        while (!decoders_[0]->frame_queue().isComplete() &&
        //while (std::all_of(decoders_.begin(), decoders_.end(), [](auto &d) {return !d->frame_queue().isComplete(); }) &&
                remaining--) {
            std::vector<Frame> decodedFrames;
            auto dropOrDuplicate = alignment.dropOrDuplicate(framesDecoded++, framesEncoded);

            for(auto &decodeSession: decodeSessions)
                if(!decodeSession->decoder().frame_queue().isComplete())
                    decodedFrames.emplace_back(decodeSession->decode());

            auto processedFrame = transform(lock_, decodedFrames);

            for (auto i = 0u; i <= dropOrDuplicate; i++, framesEncoded++)
                encodeSession.Encode(processedFrame);
        }

        LOG(INFO) << "Union complete (decoded " << framesDecoded << ", encoded " << framesEncoded << ")";
    }

    VideoEncoder &encoder() { return encoder_; }
    const std::vector<std::shared_ptr<CudaDecoder>> &decoders() const { return decoders_; }

private:
    VideoLock lock_;
    CUVIDFrameQueue frameQueue_;
    VideoEncoder encoder_;
    std::vector<std::shared_ptr<CudaDecoder>> decoders_;
};


#endif // LIGHTDB_UNIONTRANSCODER_H
