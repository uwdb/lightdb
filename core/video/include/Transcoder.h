#ifndef TRANSCODER_H
#define TRANSCODER_H

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "EncodeWriter.h"
#include "VideoLock.h"

class Transcoder {
public:
    Transcoder(GPUContext& context, EncodeConfig& configuration)
            : lock(context), encoder_(context, configuration, lock),
              frameQueue(lock.get()), configuration(configuration),
              decoder(configuration, frameQueue, lock),
              fpsRatio(1) { }

    int transcode(const std::string &inputFilename, EncodeWriter &writer);
    int transcode(const std::string &inputFilename, const std::string outputFilename) {
        FileEncodeWriter writer(encoder().api(), outputFilename);
        return transcode(inputFilename, writer);
    }

    VideoEncoder &encoder() { return encoder_; }

private:
    VideoLock lock;
    CUVIDFrameQueue frameQueue;
    VideoEncoder encoder_;
    CudaDecoder decoder;
    EncodeConfig& configuration;
    float fpsRatio;
};


#endif
