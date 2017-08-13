#ifndef _VIDEO_ENCODER
#define _VIDEO_ENCODER

#include <vector>
#include "EncodeBuffer.h"
#include "EncodeAPI.h"
#include "VideoLock.h"

class VideoEncoder {
public:
  VideoEncoder(GPUContext& context, EncodeConfig& configuration, VideoLock& lock)
          : configuration(configuration), api_(context), lock(lock), buffers(CreateBuffers(minimumBufferCount())) {
      //TODO not the right place to set this.
      configuration.presetGUID = api().GetPresetGUID(configuration.encoderPreset.c_str(), configuration.codec);
  }

  EncodeAPI &api() { return api_; }

protected:
  EncodeConfig& configuration; //TODO const?
  EncodeAPI api_;
  VideoLock &lock;
  std::vector<EncodeBuffer> buffers;

  friend class VideoEncoderSession;

  size_t minimumBufferCount() const { return configuration.numB + 4; }

private:
  std::vector<EncodeBuffer> CreateBuffers(const size_t size) {
      std::vector<EncodeBuffer> buffers;
      while(buffers.size() <= size)
          buffers.emplace_back(api(), configuration);
      return buffers;
  }
};

#endif
