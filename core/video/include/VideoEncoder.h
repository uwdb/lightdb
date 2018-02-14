#ifndef LIGHTDB_VIDEOENCODER_H
#define LIGHTDB_VIDEOENCODER_H

#include "EncodeAPI.h"
#include "Configuration.h"
#include "VideoLock.h"
#include <memory>
#include <mutex>
#include <vector>

struct EncodeBuffer;

class VideoEncoder {
public:
  VideoEncoder(VideoEncoder &&other) = delete;

  VideoEncoder(GPUContext& context, const EncodeConfiguration& configuration, VideoLock& lock)
          : configuration_(configuration), context_(context), api_(context), lock(lock),
            encoderHandle_(VideoEncoderHandle(context, api_, configuration)),
            buffers(CreateBuffers(minimumBufferCount())) {
      if(api().ValidatePresetGUID(configuration) != NV_ENC_SUCCESS)
          throw InvalidArgumentError("Invalid preset guid", "configuration");
  }

  EncodeAPI &api() { return api_; }
  const EncodeConfiguration &configuration() const { return configuration_; }

protected:
    const EncodeConfiguration& configuration_;
    GPUContext &context_;
    EncodeAPI api_;
    VideoLock &lock;

private:
    class VideoEncoderHandle {
    public:
        VideoEncoderHandle(GPUContext &context, EncodeAPI &api, const EncodeConfiguration &configuration)
                : context_(context), api_(api), configuration_(configuration) {
            NVENCSTATUS status;

            if((status = api_.CreateEncoder(&configuration_)) != NV_ENC_SUCCESS)
                throw GpuEncodeRuntimeError("Call to api.CreateEncoder failed", status);
        }

        ~VideoEncoderHandle() {
            NVENCSTATUS result;
            if((result = api_.NvEncDestroyEncoder()) != NV_ENC_SUCCESS)
                LOG(ERROR) << "Swallowed failure to destroy encoder (NVENCSTATUS " << result << ") in destructor";
        }

    private:
        GPUContext &context_;
        EncodeAPI &api_;
        const EncodeConfiguration &configuration_;
    } encoderHandle_;

  std::vector<std::shared_ptr<EncodeBuffer>> buffers;

  friend class VideoEncoderSession;

  size_t minimumBufferCount() const { return configuration().numB + 4; }

private:
  std::vector<std::shared_ptr<EncodeBuffer>> CreateBuffers(size_t size);
};

#endif // LIGHTDB_VIDEOENCODER_H
