#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <optional>
#include <string>

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "DecoderLock.h"
#include "dynlink_cuda.h"

class Transcoder2 {
public:
    Transcoder2(CUcontext context, EncodeConfig& configuration);
    ~Transcoder2() {
        //if (encoder != nullptr)
            encoder.Deinitialize();

        //delete encoder;
        //delete frameQueue;

        //if(lock != nullptr)
//            cuvidCtxLockDestroy(lock);
    }

    int transcode(const std::string &inputFilename, const std::string &outputFilename);

private:
    int InitializeEncoder(const std::string &outputFilename);
    void InitializeDecoder(const std::string &inputFilename);
    //static CUvideoctxlock CreateLock(CUcontext context);

    std::string inputFilename;
    std::string outputFilename;
    const std::string preset;

    CUcontext context; // shared ptr
    DecoderLock lock;
    VideoEncoder encoder;
    CudaDecoder decoder;
    CUVIDFrameQueue frameQueue;
    EncodeConfig& configuration;
    float fpsRatio;
};


class Transcoder {
public:
  static std::optional<Transcoder> create(unsigned int height, unsigned int width, unsigned int codec,
                                          std::string preset, unsigned int fps, unsigned int gop_length,
                                          unsigned long bitrate, unsigned int rcmode, unsigned int deviceId)
  {
      Transcoder transcoder(height, width, codec, preset, fps, gop_length, bitrate, rcmode, deviceId);
      transcoder.initialize();
      return std::optional<Transcoder>(transcoder);
  }

  Transcoder(unsigned int height, unsigned int width, unsigned int codec, std::string preset, unsigned int fps,
             unsigned int gop_length, unsigned long bitrate, unsigned int rcmode, unsigned int deviceId);
  ~Transcoder() {
    if (encoder != nullptr)
      encoder->Deinitialize();

    delete encoder;
    delete frameQueue;

    if(lock != nullptr)
      cuvidCtxLockDestroy(lock);
    if(context != nullptr)
      cuCtxDestroy(context);
  }

  int initialize();
  int transcode(const std::string &inputFilename, const std::string &outputFilename);

private:
  int InitializeEncoder(const std::string &outputFilename);
  void InitializeDecoder(const std::string &inputFilename);

  std::string inputFilename;
  std::string outputFilename;
  const std::string preset;

  CUcontext context;
  CUvideoctxlock lock;
  VideoEncoder *encoder; // TODO
  CudaDecoder decoder;
  FrameQueue *frameQueue; // TODO
  EncodeConfig configuration;
  float fpsRatio;
};

#endif
