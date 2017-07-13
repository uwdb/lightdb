#ifndef VISUALCLOUD_VIDEOENCODESESSION_H
#define VISUALCLOUD_VIDEOENCODESESSION_H

#include "nvEncodeAPI.h"
#include "VideoEncoder.h"
#include "EncodeWriter.h"

class VideoEncoder;

class VideoEncoderSession {
public:
    VideoEncoderSession(VideoEncoder &encoder, EncodeWriter &writer)
            : encoder(encoder), writer(writer), encodedFrameCount(0) {
        encoder.AllocateIOBuffers();
    }
    ~VideoEncoderSession() {
        Flush();
    }

    NVENCSTATUS Encode(EncodeFrameConfig&, NV_ENC_PIC_STRUCT type = NV_ENC_PIC_STRUCT_FRAME);
    NVENCSTATUS Flush();

    size_t GetEncodedFrames() const { return encodedFrameCount; }

protected:
    size_t encodedFrameCount;

    VideoEncoder &encoder;
    EncodeWriter &writer;

private:
    EncodeBuffer *CompletePendingBuffer();
    EncodeBuffer *GetAvailableBuffer();
};


#endif //VISUALCLOUD_VIDEOENCODESESSION_H
