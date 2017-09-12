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


class CudaDecoder: public VideoDecoder {
public:
  CudaDecoder(const EncodeConfig &configuration, FrameQueue& frameQueue, VideoLock& lock);
  virtual ~CudaDecoder();

  const CUvideodecoder handle() const { return handle_; }

  const CUVIDDECODECREATEINFO &parameters() const { return parameters_; }

protected:
  CUvideodecoder handle_;
  CUVIDDECODECREATEINFO parameters_;
};

#endif
