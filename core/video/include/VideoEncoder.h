#ifndef _VIDEO_ENCODER
#define _VIDEO_ENCODER

#include "EncodeBuffer.h"
#include "EncodeAPI.h"
#include "VideoLock.h"
#include <memory>
#include <mutex>
#include <vector>

class VideoEncoder {
public:
  VideoEncoder(VideoEncoder &&other) = delete;

  VideoEncoder(GPUContext& context, EncodeConfig& configuration, VideoLock& lock)
          : configuration_(configuration), context_(context), api_(context), lock(lock), encoderHandle_(VideoEncoderHandle(context, api_, configuration)), buffers(CreateBuffers(minimumBufferCount())) {
      //TODO not the right place to set this.
      configuration.presetGUID = api().GetPresetGUID(configuration.encoderPreset.c_str(), configuration.codec);
  }

  EncodeAPI &api() { return api_; }
  const EncodeConfig &configuration() const { return configuration_; }

protected:
    EncodeConfig& configuration_; //TODO const?
    GPUContext &context_;
    EncodeAPI api_;
    VideoLock &lock;

private:
    class VideoEncoderHandle {
    public:
        VideoEncoderHandle(GPUContext &context, EncodeAPI &api, EncodeConfig &configuration)
                : context_(context), api_(api), configuration_(configuration) {
            NVENCSTATUS status;

            //if((status = api_.NvEncOpenEncodeSessionEx(context_.get(), NV_ENC_DEVICE_TYPE_CUDA)) != NV_ENC_SUCCESS)
              //  throw std::runtime_error("foo"); // status; //TODO
            /*else*/ if((status = api_.CreateEncoder(&configuration_)) != NV_ENC_SUCCESS)
                throw status; //TODO
        }

        ~VideoEncoderHandle() {
            api_.NvEncDestroyEncoder();
        }

    private:
        EncodeConfig &configuration_;
        GPUContext &context_;
        EncodeAPI &api_;
    } encoderHandle_;

  std::vector<std::shared_ptr<EncodeBuffer>> buffers;

  friend class VideoEncoderSession;

  size_t minimumBufferCount() const { return configuration().numB + 4; }

    /*
    int i = 0;
  VideoEncoder &tempdeleteme() {
      i++;
      if(i != 1) {
          api().NvEncDestroyEncoder();
          api().NvEncOpenEncodeSessionEx(context_.get(), NV_ENC_DEVICE_TYPE_CUDA);
          api().CreateEncoder(&configuration_);
          //api_ = *new EncodeAPI(context_);
          //buffers = CreateBuffers(buffers.size());
      }
      return *this;
  }*/

private:
  std::vector<std::shared_ptr<EncodeBuffer>> CreateBuffers(const size_t size) {
      std::vector<std::shared_ptr<EncodeBuffer>> buffers;

      buffers.reserve(size);
      std::generate_n(std::back_inserter(buffers), size,
                      [this]() { return std::make_shared<EncodeBuffer>(this->api(), this->configuration()); });
      //while(buffers.size() <= size)
      //    buffers.emplace_back(api(), configuration());
      return buffers;
  }
};

#endif
