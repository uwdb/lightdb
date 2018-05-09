#ifndef LIGHTDB_VIDEODECODER_H
#define LIGHTDB_VIDEODECODER_H

#include <errors.h>
#include "Configuration.h"
#include "FrameQueue.h"
#include "VideoLock.h"
#include "dynlink_cuda.h"
#include "dynlink_nvcuvid.h"

class VideoDecoder {
public:
    const DecodeConfiguration& configuration() const { return configuration_; }
    FrameQueue &frame_queue() const {return frame_queue_; }

protected:
    //TODO push frame_queue into session
    VideoDecoder(const DecodeConfiguration &configuration, FrameQueue& frame_queue)
        : configuration_(configuration), frame_queue_(frame_queue)
    { }

private:
    const DecodeConfiguration &configuration_;
    FrameQueue& frame_queue_;
};


class CudaDecoder: public VideoDecoder {
public:
  CudaDecoder(const DecodeConfiguration &configuration, FrameQueue& frame_queue, VideoLock& lock)
          : VideoDecoder(configuration, frame_queue), handle_(nullptr)
  {
      CUresult result;
      auto decoderCreateInfo = configuration.AsCuvidCreateInfo(lock);

      if((result = cuvidCreateDecoder(&handle_, &decoderCreateInfo)) != CUDA_SUCCESS) {
          throw GpuCudaRuntimeError("Call to cuvidCreateDecoder failed", result);
      }
  }

  CudaDecoder(const CudaDecoder&) = delete;
  CudaDecoder(CudaDecoder&& other) noexcept
          : VideoDecoder(other),
            handle_(other.handle_) {
      other.handle_ = nullptr;
  }

  virtual ~CudaDecoder() {
      if(handle() != nullptr)
          cuvidDestroyDecoder(handle());
  }

  const CUvideodecoder handle() const { return handle_; }

protected:
  CUvideodecoder handle_;
};

#endif // LIGHTDB_VIDEODECODER_H
