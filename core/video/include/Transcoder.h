#ifndef TRANSCODER_H
#define TRANSCODER_H

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "VideoEncoderSession.h"
#include "VideoDecoderSession.h"
#include "DecodeReader.h"
#include "EncodeWriter.h"
#include "FrameRateAlignment.h"


class Transcoder {
public:
    Transcoder(GPUContext& context,
               DecodeConfiguration &decodeConfiguration,
               EncodeConfiguration &encodeConfiguration)
            : lock_(context), frameQueue_(lock_.get()),
              encoder_(context, encodeConfiguration, lock_),
              decoder_(decodeConfiguration, frameQueue_, lock_)
    { }

    NVENCSTATUS transcode(DecodeReader &reader, EncodeWriter &writer) {
        NVENCSTATUS status;
        VideoDecoderSession decodeSession(decoder_, reader);
        VideoEncoderSession encodeSession(encoder_, writer);
        FrameRateAlignment alignment(encoder_.configuration().framerate, decoder_.configuration().framerate);
        size_t framesDecoded = 0, framesEncoded = 0;

        while (!decoder_.frame_queue().isComplete()) {
            auto dropOrDuplicate = alignment.dropOrDuplicate(framesDecoded++, framesEncoded);
            auto frame = decodeSession.decode();

            for (auto i = 0; i <= dropOrDuplicate; i++, framesEncoded++)
                if((status = encodeSession.Encode(frame)) != NV_ENC_SUCCESS)
                    return status;
        }

        return NV_ENC_SUCCESS;
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
