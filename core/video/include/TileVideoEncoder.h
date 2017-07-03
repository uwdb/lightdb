#ifndef _TILE_VIDEO_ENCODER
#define _TILE_VIDEO_ENCODER

#include <deque>
#include <vector>
#include <algorithm>

#include "VideoEncoder.h"
#include "dynlink_nvcuvid.h"
#include <cassert>

typedef struct TileEncodeContext {
  EncodeAPI& hardwareEncoder;
  EncodeConfig& configuration;
  std::vector<EncodeBuffer> encodeBuffer;
  //EncodeBuffer encodeBuffer[MAX_ENCODE_QUEUE];
  CNvQueue<EncodeBuffer> encodeBufferQueue;
  size_t offsetX, offsetY;

  TileEncodeContext(EncodeAPI& api, EncodeConfig& configuration)
    : hardwareEncoder(api), configuration(configuration), encodeBufferQueue(), offsetX(0), offsetY(0) {

    std::generate_n(std::back_inserter(encodeBuffer),
                    MAX_ENCODE_QUEUE,
                    [&api, &configuration]() -> EncodeBuffer&& { return EncodeBuffer(api, configuration); });
    //encodeBuffer.reserve(MAX_ENCODE_QUEUE);
    //for(auto i = 0; i < MAX_ENCODE_QUEUE; i++)
    //    encodeBuffer.emplace_back(EncodeBuffer(api, configuration));
  }
} TileEncodeContext;

typedef struct TileDimensions {
  size_t rows;
  size_t columns;
  size_t count;
} TileDimensions;

class TileVideoEncoder {
public:
    //TODO use tile dimensions in configuration
  TileVideoEncoder(EncodeAPI& api, CUvideoctxlock lock, EncodeConfig& configuration, const unsigned int tileColumns, const unsigned int tileRows)
      : tileDimensions({tileRows, tileColumns, tileColumns * tileRows}),
        lock(lock), encodeBufferSize(0), framesEncoded(0) {
    assert(tileColumns > 0 && tileRows > 0);

    std::generate_n(std::back_inserter(tileEncodeContext),
                    tileDimensions.count,
                    [&api, &configuration]() -> TileEncodeContext&& { return TileEncodeContext(api, configuration); });
  }
  virtual ~TileVideoEncoder() {  ReleaseIOBuffers(); }

  //NVENCSTATUS Initialize(void *, const NV_ENC_DEVICE_TYPE);
  NVENCSTATUS CreateEncoders(EncodeConfig &);
  //NVENCSTATUS Deinitialize();
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
