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
    VideoEncoder(GPUContext& context,
                 const EncodeConfiguration& configuration,
                 VideoLock& lock,
                 NV_ENC_BUFFER_FORMAT input_format=NV_ENC_BUFFER_FORMAT_NV12_PL)
            : configuration_(configuration), context_(context), api_(std::make_shared<EncodeAPI>(context)), lock_(lock),
              encoderHandle_(VideoEncoderHandle(context, *api_, configuration)),
              input_format_(input_format),
              buffers(CreateBuffers(minimumBufferCount(), input_format)) {
        if(api().ValidatePresetGUID(configuration) != NV_ENC_SUCCESS)
            throw InvalidArgumentError("Invalid preset guid", "configuration");
    }

  VideoEncoder(VideoEncoder &) = delete;
  //TODO this is inefficient, we really want to use the existing buffers, but they refer to the old encoder :(
  VideoEncoder(VideoEncoder &&other) noexcept
            : configuration_(other.configuration_), context_(other.context_), api_(other.api_), lock_(other.lock_),
              encoderHandle_(std::move(other.encoderHandle_)),
              input_format_(other.input_format()),
              buffers(CreateBuffers(minimumBufferCount(), input_format_))
    { }

  EncodeAPI &api() { return *api_; }
  NV_ENC_BUFFER_FORMAT input_format() const { return input_format_; }
  const EncodeConfiguration &configuration() const { return configuration_; }

protected:
    const EncodeConfiguration& configuration_;
    GPUContext &context_;
    std::shared_ptr<EncodeAPI> api_;
    VideoLock &lock_;

private:
    class VideoEncoderHandle {
    public:
        VideoEncoderHandle(GPUContext &context, EncodeAPI &api, const EncodeConfiguration &configuration)
                : context_(context), api_(api), configuration_(configuration) {
            NVENCSTATUS status;

            if((status = api_.CreateEncoder(&configuration_)) != NV_ENC_SUCCESS)
                throw GpuEncodeRuntimeError("Call to api.CreateEncoder failed", status);
        }

        VideoEncoderHandle(VideoEncoderHandle &) = delete;
        VideoEncoderHandle(VideoEncoderHandle &&other) noexcept
                : context_(other.context_), api_(other.api_), configuration_(other.configuration_) {
            other.moved_ = true; }

        ~VideoEncoderHandle() {
            NVENCSTATUS result;
            if(!moved_ && (result = api_.NvEncDestroyEncoder()) != NV_ENC_SUCCESS)
                LOG(ERROR) << "Swallowed failure to destroy encoder (NVENCSTATUS " << result << ") in destructor";
        }

    private:
        GPUContext &context_;
        EncodeAPI &api_;
        const EncodeConfiguration &configuration_;
        bool moved_ = false;
    } encoderHandle_;

  NV_ENC_BUFFER_FORMAT input_format_;
  std::vector<std::shared_ptr<EncodeBuffer>> buffers;

  friend class VideoEncoderSession;

  size_t minimumBufferCount() const { return configuration().numB + 4; }

private:
  std::vector<std::shared_ptr<EncodeBuffer>> CreateBuffers(size_t, NV_ENC_BUFFER_FORMAT);
};

#endif // LIGHTDB_VIDEOENCODER_H
