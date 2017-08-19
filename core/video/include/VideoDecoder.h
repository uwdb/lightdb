#ifndef _VIDEO_DECODER
#define _VIDEO_DECODER

#include "EncodeAPI.h"
#include "FrameQueue.h"
#include "VideoLock.h"
#include "dynlink_cuda.h"
#include "dynlink_nvcuvid.h"

class VideoDecoder {
public:
    const EncodeConfig& configuration() const { return configuration_; }
    FrameQueue &frame_queue() const {return frame_queue_; }

protected:
    //TODO push frame_queue into session
    VideoDecoder(const EncodeConfig &configuration, FrameQueue& frame_queue)
        : configuration_(configuration), frame_queue_(frame_queue)
    { }

private:
    const EncodeConfig &configuration_;
    FrameQueue& frame_queue_;
};


//TODO VideoDecoder base class, CudaDecoder concrete implementation
class CudaDecoder: public VideoDecoder {
public:
  CudaDecoder(const EncodeConfig &configuration, FrameQueue& frameQueue, VideoLock& lock);
  virtual ~CudaDecoder();

  virtual void Start();

    //TODO this should move into a session
  virtual void GetCodecParam(int *width, int *height, int *frame_rate_num, int *frame_rate_den, int *is_progressive);

  const CUvideodecoder handle() const { return handle_; }
  size_t decodedFrameCount() const { return decodedFrameCount_; }
  bool complete() { return complete_; }

protected:
  CUvideosource m_videoSource;

public: //TODO these should be protected
    CUvideoparser m_videoParser;
    CUVIDDECODECREATEINFO m_oVideoDecodeCreateInfo;
    size_t decodedFrameCount_;

protected:

protected:
  CUvideodecoder handle_;
  bool complete_;
  VideoLock& lock;
};

#endif
