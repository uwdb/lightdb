#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <optional>
#include <string>

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "DecoderLock.h"
#include "dynlink_cuda.h"

class Transcoder {
public:
    Transcoder(GPUContext& context, EncodeConfig& configuration);

    int transcode(const std::string &inputFilename, const std::string &outputFilename);

private:
    int InitializeEncoder(const std::string &outputFilename);

    DecoderLock lock;
    CUVIDFrameQueue frameQueue;
    VideoEncoder encoder;
    CudaDecoder decoder;
    EncodeConfig& configuration;
    float fpsRatio;
};


#endif
