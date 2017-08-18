#ifndef _VIDEO_DECODER
#define _VIDEO_DECODER

#include "EncodeAPI.h"
#include "FrameQueue.h"
#include "VideoLock.h"
#include "dynlink_cuda.h"
#include "dynlink_nvcuvid.h"

//TODO rename to VideoDecoder
class CudaDecoder {
public:
  CudaDecoder(const EncodeConfig &configuration, FrameQueue& frameQueue, VideoLock& lock);
  virtual ~CudaDecoder();

  virtual void Start();

    //TODO this should move into a session
  virtual void GetCodecParam(int *width, int *height, int *frame_rate_num, int *frame_rate_den, int *is_progressive);

  const CUvideodecoder handle() const { return handle_; }
  const EncodeConfig& configuration() const { return configuration_; }
  size_t decodedFrameCount() const { return decodedFrameCount_; }
  bool complete() { return complete_; }

protected:
  CUvideosource m_videoSource;

  const EncodeConfig& configuration_;

public: //TODO these should be protected
    FrameQueue& frameQueue;
    CUvideoparser m_videoParser;
    CUVIDDECODECREATEINFO m_oVideoDecodeCreateInfo;
    size_t decodedFrameCount_;

protected:
  virtual void InitVideoDecoder(const std::string &inputFilename); //TODO remove deprecated overload
  virtual void InitVideoDecoder(CUVIDEOFORMAT&);

protected:
  CUvideodecoder handle_;
  bool complete_;
  VideoLock& lock;
};

#endif
