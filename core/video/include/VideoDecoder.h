#ifndef _VIDEO_DECODER
#define _VIDEO_DECODER

#include "EncodeAPI.h"
#include "FrameQueue.h"
#include "DecoderLock.h"
#include "dynlink_cuda.h"    // <cuda.h>
#include "dynlink_nvcuvid.h" // <nvcuvid.h>

class CudaDecoder {
public:
  CudaDecoder(const EncodeConfig &configuration, FrameQueue& frameQueue, DecoderLock& lock);
  virtual ~CudaDecoder(void);

  bool IsFinished() { return m_bFinish; }
  virtual void InitVideoDecoder(const std::string &inputFilename);
  virtual void Start();
  virtual void GetCodecParam(int *width, int *height, int *frame_rate_num, int *frame_rate_den, int *is_progressive);
  virtual void *GetDecoder() { return m_videoDecoder; }

public:
  CUvideosource m_videoSource;
  CUvideoparser m_videoParser;
  CUvideodecoder m_videoDecoder;
  CUVIDDECODECREATEINFO m_oVideoDecodeCreateInfo;

  FrameQueue& frameQueue;
  const EncodeConfig& configuration;

  int m_decodedFrames;

  void Deinitialize();

protected:
  bool m_bFinish;
  DecoderLock& lock;
};

#endif
