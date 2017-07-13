#ifndef _VIDEO_ENCODER
#define _VIDEO_ENCODER

#include <vector>
#include "EncodeBuffer.h"
#include "EncodeAPI.h"
#include "VideoLock.h"

class VideoEncoder {
public:
  VideoEncoder(GPUContext& context, EncodeConfig& configuration, VideoLock& lock)
          : configuration(configuration), api_(context), lock(lock) {
      //TODO not the right place to set this.
      configuration.presetGUID = api().GetPresetGUID(configuration.encoderPreset.c_str(), configuration.codec);

      while(buffers.size() <= minimumBufferCount())
          buffers.emplace_back(api(), configuration);
  }

  EncodeAPI &api() { return api_; }

protected:
  EncodeConfig& configuration;
  EncodeAPI api_;
  VideoLock &lock;
  std::vector<EncodeBuffer> buffers;

  friend class VideoEncoderSession;

  size_t minimumBufferCount() const { return configuration.numB + 4; }
};

#endif
