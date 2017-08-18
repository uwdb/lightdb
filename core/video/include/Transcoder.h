#ifndef TRANSCODER_H
#define TRANSCODER_H

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "VideoEncoderSession.h"
#include "VideoDecoderSession.h"
#include "DecodeReader.h"
#include "EncodeWriter.h"


class Transcoder {
public:
    Transcoder(GPUContext& context, EncodeConfig& configuration)
            : lock_(context), frameQueue_(lock_.get()),
              encoder_(context, configuration, lock_),
              decoder_(configuration, frameQueue_, lock_)
    { }

    NVENCSTATUS transcode(DecodeReader &reader, EncodeWriter &writer) {
        NVENCSTATUS status;
        size_t framesDecoded = 0, framesEncoded = 0;
        VideoDecoderSession decodeSession(decoder_, reader);
        VideoEncoderSession encodeSession(encoder_, writer);
        //TODO push this into a broad encode/decode configuration struct
        auto fpsRatio = (float)encoder_.configuration().fps /
                        reader.format().frame_rate.numerator / reader.format().frame_rate.denominator;

        while (!decoder_.frameQueue.isComplete()) {
            auto frame = decodeSession.decode();
            auto dropOrDuplicate = alignFPS(fpsRatio, framesDecoded++, framesEncoded);

            for (auto i = 0; i <= dropOrDuplicate; i++, framesEncoded++)
                if((status = encodeSession.encode(frame)) != NV_ENC_SUCCESS)
                    return status;
        }

        encodeSession.Flush();

        return NV_ENC_SUCCESS;
    }

    VideoEncoder &encoder() { return encoder_; }

private:
    VideoLock lock_;
    CUVIDFrameQueue frameQueue_;
    VideoEncoder encoder_;
    CudaDecoder decoder_;

    //TODO push this into a broad encode/decode configuration struct
    int alignFPS(const float fpsRatio, const size_t decodedFrames, const size_t encodedFrames) {
        if (fpsRatio < 1.f) {
            // need to drop frame
            return decodedFrames * fpsRatio < (encodedFrames + 1) ? -1 : 0;
        } else if (fpsRatio > 1.f) {
            // need to duplicate frame
            auto duplicate = 0;
            while (decodedFrames * fpsRatio > encodedFrames + duplicate + 1) {
                duplicate++;
            }

            return duplicate;
        } else {
            return 0;
        }
    }
};


#endif
