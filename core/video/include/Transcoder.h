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
    ~Transcoder() {
        encoder.Deinitialize();
    }

    int transcode(const std::string &inputFilename, const std::string &outputFilename);

private:
    int InitializeEncoder(const std::string &outputFilename);
    void InitializeDecoder(const std::string &inputFilename);

    std::string inputFilename;
    std::string outputFilename;
    const std::string preset;

    DecoderLock lock;
    VideoEncoder encoder;
    CudaDecoder decoder;
    CUVIDFrameQueue frameQueue;
    EncodeConfig& configuration;
    float fpsRatio;
};


#endif
