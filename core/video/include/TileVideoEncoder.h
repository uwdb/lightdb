#ifndef _TILE_VIDEO_ENCODER
#define _TILE_VIDEO_ENCODER

#include <deque>
#include <vector>
#include <algorithm>

#include "VideoEncoderSession.h"
#include "dynlink_nvcuvid.h"
#include <cassert>

#define MAX_ENCODE_QUEUE 32

//TODO see updated EncodeBufferQueue in VideoEncoderSession.h
template <class T> class CNvQueue {
    T **m_pBuffer;
    size_t m_uSize;
    size_t m_uPendingCount;
    size_t m_uAvailableIdx;
    size_t m_uPendingndex;

public:
    CNvQueue() : m_pBuffer(NULL), m_uSize(0), m_uPendingCount(0), m_uAvailableIdx(0), m_uPendingndex(0) {}

    ~CNvQueue() { delete[] m_pBuffer; }

    // TODO make RAII, should take vector of items
    bool Initialize(T *pItems, size_t uSize) {
      m_uSize = uSize;
      m_uPendingCount = 0;
      m_uAvailableIdx = 0;
      m_uPendingndex = 0;
      m_pBuffer = new T *[m_uSize];
      for (size_t i = 0; i < m_uSize; i++) {
        m_pBuffer[i] = &pItems[i];
      }
      return true;
    }

    T *GetAvailable() {
      T *pItem = NULL;
      if (m_uPendingCount == m_uSize) {
        return NULL;
      }
      pItem = m_pBuffer[m_uAvailableIdx];
      m_uAvailableIdx = (m_uAvailableIdx + 1) % m_uSize;
      m_uPendingCount += 1;
      return pItem;
    }

    T *GetPending() {
      if (m_uPendingCount == 0) {
        return NULL;
      }

      T *pItem = m_pBuffer[m_uPendingndex];
      m_uPendingndex = (m_uPendingndex + 1) % m_uSize;
      m_uPendingCount -= 1;
      return pItem;
    }
};




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
  NVENCSTATUS CreateEncoders(const std::string &filenameTemplate, EncodeConfig &);
  //NVENCSTATUS Deinitialize();
  NVENCSTATUS
  EncodeFrame(Frame *, const NV_ENC_PIC_STRUCT type = NV_ENC_PIC_STRUCT_FRAME, const bool flush = false);
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
