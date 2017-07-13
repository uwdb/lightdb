#ifndef _VIDEO_DECODER
#define _VIDEO_DECODER

#include "EncodeAPI.h"
#include "FrameQueue.h"
#include "VideoLock.h"
#include "dynlink_cuda.h"    // <cuda.h>
#include "dynlink_nvcuvid.h" // <nvcuvid.h>

class CudaDecoder {
public:
  CudaDecoder(const EncodeConfig &configuration, FrameQueue& frameQueue, VideoLock& lock);
  virtual ~CudaDecoder();

  virtual void InitVideoDecoder(const std::string &inputFilename);
  virtual void Start();
  virtual void GetCodecParam(int *width, int *height, int *frame_rate_num, int *frame_rate_den, int *is_progressive);

  CUvideodecoder handle() { return handle_; }
  bool complete() { return complete_; }

public:
  CUvideosource m_videoSource;
  CUvideoparser m_videoParser;
  CUvideodecoder handle_;
  CUVIDDECODECREATEINFO m_oVideoDecodeCreateInfo;

  FrameQueue& frameQueue;
  const EncodeConfig& configuration;

  int m_decodedFrames;

  void Deinitialize();

protected:
  bool complete_;
  VideoLock& lock;
};

#endif
