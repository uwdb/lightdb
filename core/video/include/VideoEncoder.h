
#ifndef _VIDEO_ENCODER
#define _VIDEO_ENCODER

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "FrameQueue.h"
#include "EncodeBuffer.h"
#include "EncodeAPI.h"
#include "nvEncodeAPI.h"

#include "dynlink_cuda.h"    // <cuda.h>
#include "dynlink_nvcuvid.h" // <nvcuvid.h>

#define __cu(a)                                                                                                        \
  do {                                                                                                                 \
    CUresult ret;                                                                                                      \
    if ((ret = (a)) != CUDA_SUCCESS) {                                                                                 \
      fprintf(stderr, "%s has returned CUDA error %d\n", #a, ret);                                                     \
      return NV_ENC_ERR_GENERIC;                                                                                       \
    }                                                                                                                  \
  } while (0)

#define MAX_ENCODE_QUEUE 32

#define SET_VER(configStruct, type)                                                                                    \
  { configStruct.version = type##_VER; }

template <class T> class CNvQueue {
  T **m_pBuffer;
  unsigned int m_uSize;
  unsigned int m_uPendingCount;
  unsigned int m_uAvailableIdx;
  unsigned int m_uPendingndex;

public:
  CNvQueue() : m_pBuffer(NULL), m_uSize(0), m_uPendingCount(0), m_uAvailableIdx(0), m_uPendingndex(0) {}

  ~CNvQueue() { delete[] m_pBuffer; }

  bool Initialize(T *pItems, unsigned int uSize) {
    m_uSize = uSize;
    m_uPendingCount = 0;
    m_uAvailableIdx = 0;
    m_uPendingndex = 0;
    m_pBuffer = new T *[m_uSize];
    for (unsigned int i = 0; i < m_uSize; i++) {
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

typedef struct _EncodeFrameConfig {
  CUdeviceptr dptr;
  unsigned int pitch;
  unsigned int width;
  unsigned int height;
} EncodeFrameConfig;

class VideoEncoder {
public:
  VideoEncoder(CUvideoctxlock ctxLock);
  virtual ~VideoEncoder();

protected:
  EncodeAPI *m_pNvHWEncoder;
  CUvideoctxlock m_ctxLock;
  uint32_t m_uEncodeBufferCount;
  EncodeBuffer m_stEncodeBuffer[MAX_ENCODE_QUEUE];
  CNvQueue<EncodeBuffer> m_EncodeBufferQueue;

  EncodeConfig m_stEncoderInput;
  EncodeOutputBuffer m_stEOSOutputBfr;

  int32_t m_iEncodedFrames;

public:
  EncodeAPI *GetHWEncoder() { return m_pNvHWEncoder; }
  NVENCSTATUS Deinitialize();
  NVENCSTATUS EncodeFrame(EncodeFrameConfig *pEncodeFrame, NV_ENC_PIC_STRUCT picType = NV_ENC_PIC_STRUCT_FRAME,
                          bool bFlush = false);
  NVENCSTATUS AllocateIOBuffers(EncodeConfig *pEncodeConfig);
  int32_t GetEncodedFrames() { return m_iEncodedFrames; }

  NVENCSTATUS ReleaseIOBuffers();

protected:
  NVENCSTATUS FlushEncoder();
};

// NVEncodeAPI entry point
typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST *);

#endif
