#ifndef _TILE_VIDEO_ENCODER
#define _TILE_VIDEO_ENCODER

#include <vector>

#include "VideoEncoder.h"
#include "dynlink_nvcuvid.h" // <nvcuvid.h>
#include <cassert>

typedef struct TileEncodeContext {
  CNvHWEncoder hardwareEncoder;
  EncodeBuffer encodeBuffer[MAX_ENCODE_QUEUE];
  CNvQueue<EncodeBuffer> encodeBufferQueue;
  size_t offsetX, offsetY;
} TileEncodeContext;

typedef struct TileDimensions {
  size_t rows;
  size_t columns;
  size_t count;
} TileDimensions;

class TileVideoEncoder {
public:
  TileVideoEncoder(CUvideoctxlock lock, const unsigned int tileColumns, const unsigned int tileRows)
      : tileDimensions({tileRows, tileColumns, tileColumns * tileRows}), tileEncodeContext(tileDimensions.count),
        lock(lock), encodeBufferSize(0), framesEncoded(0) {
    assert(tileColumns > 0 && tileRows > 0);
  }
  virtual ~TileVideoEncoder() {}

  NVENCSTATUS Initialize(void *, const NV_ENC_DEVICE_TYPE);
  NVENCSTATUS CreateEncoders(EncodeConfig &);
  NVENCSTATUS Deinitialize();
  NVENCSTATUS
  EncodeFrame(EncodeFrameConfig *, const NV_ENC_PIC_STRUCT type = NV_ENC_PIC_STRUCT_FRAME, const bool flush = false);
  NVENCSTATUS AllocateIOBuffers(const EncodeConfig *);
  size_t GetEncodedFrames() const { return framesEncoded; }
  GUID GetPresetGUID() const { return presetGUID; }

protected:
  GUID presetGUID;
  TileDimensions tileDimensions;
  std::vector<TileEncodeContext> tileEncodeContext;
  CUvideoctxlock lock;

  size_t encodeBufferSize;
  size_t framesEncoded;

private:
  NVENCSTATUS AllocateIOBuffer(TileEncodeContext &, const EncodeConfig &);
  NVENCSTATUS ReleaseIOBuffers();
  NVENCSTATUS FlushEncoder();
};

#endif
