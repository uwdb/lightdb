#include "VideoEncoder.h"
#include "dynlink_cuda.h" // <cuda.h>

#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024

VideoEncoder::VideoEncoder(GPUContext& context, EncodeConfig& configuration, VideoLock &lock)
        : configuration(configuration), api_(context), lock(lock) {
  //m_pNvHWEncoder = new EncodeAPI(context.get());
  //m_ctxLock = ctxLock;
  configuration.presetGUID = api().GetPresetGUID(configuration.encoderPreset.c_str(), configuration.codec);

  m_uEncodeBufferCount = 0;
  m_iEncodedFrames = 0;
  memset(&m_stEncoderInput, 0, sizeof(m_stEncoderInput));
  memset(&m_stEOSOutputBfr, 0, sizeof(m_stEOSOutputBfr));
  //memset(&m_stEncodeBuffer, 0, sizeof(m_stEncodeBuffer));
}

VideoEncoder::~VideoEncoder(void) {
    FlushEncoder();
  ReleaseIOBuffers();
}

NVENCSTATUS VideoEncoder::AllocateIOBuffers() {
  NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    if(isIOBufferAllocated)
        return NV_ENC_SUCCESS;

    isIOBufferAllocated = true;
  m_uEncodeBufferCount = configuration.numB + 4;

    for(auto i = 0; i < m_uEncodeBufferCount; i++)
        buffers.emplace_back(api(), configuration);


  //uint32_t uInputWidth = pEncodeConfig->width;
  //uint32_t uInputHeight = pEncodeConfig->height;
  m_EncodeBufferQueue.Initialize(&buffers[0], m_uEncodeBufferCount);
  m_stEncoderInput.enableAsyncMode = configuration.enableAsyncMode;

  m_stEOSOutputBfr.bEOSFlag = TRUE;
  if (m_stEncoderInput.enableAsyncMode) {
    nvStatus = api().NvEncRegisterAsyncEvent(&m_stEOSOutputBfr.hOutputEvent);
    if (nvStatus != NV_ENC_SUCCESS)
      return nvStatus;
  } else
    m_stEOSOutputBfr.hOutputEvent = NULL;

  return NV_ENC_SUCCESS;
}

NVENCSTATUS VideoEncoder::ReleaseIOBuffers() {
    if(!isIOBufferAllocated)
        return NV_ENC_SUCCESS;

    buffers.clear();
    isIOBufferAllocated = false;

  if (m_stEOSOutputBfr.hOutputEvent) {
    if (m_stEncoderInput.enableAsyncMode) {
      api().NvEncUnregisterAsyncEvent(m_stEOSOutputBfr.hOutputEvent);
      nvCloseFile(m_stEOSOutputBfr.hOutputEvent);
      m_stEOSOutputBfr.hOutputEvent = NULL;
    }
  }

  return NV_ENC_SUCCESS;
}

NVENCSTATUS VideoEncoder::FlushEncoder() {
    if(!isIOBufferAllocated)
        return NV_ENC_SUCCESS;

  NVENCSTATUS nvStatus = api().NvEncFlushEncoderQueue(m_stEOSOutputBfr.hOutputEvent);
  if (nvStatus != NV_ENC_SUCCESS) {
    assert(0);
    return nvStatus;
  }

  EncodeBuffer *pEncodeBuffer = m_EncodeBufferQueue.GetPending();
  while (pEncodeBuffer) {
    api().ProcessOutput(configuration.fOutput, pEncodeBuffer);
    pEncodeBuffer = m_EncodeBufferQueue.GetPending();
    // UnMap the input buffer after frame is done
    if (pEncodeBuffer && pEncodeBuffer->stInputBfr.hInputSurface) {
      nvStatus = api().NvEncUnmapInputResource(pEncodeBuffer->stInputBfr.hInputSurface);
      pEncodeBuffer->stInputBfr.hInputSurface = NULL;
    }
  }
#if defined(NV_WINDOWS)
  if (m_stEncoderInput.enableAsyncMode) {
    if (WaitForSingleObject(m_stEOSOutputBfr.hOutputEvent, 500) != WAIT_OBJECT_0) {
      assert(0);
      nvStatus = NV_ENC_ERR_GENERIC;
    }
  }
#endif
  return nvStatus;
}

NVENCSTATUS VideoEncoder::EncodeFrame(EncodeFrameConfig *pEncodeFrame, NV_ENC_PIC_STRUCT picType, bool bFlush) {
  NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

  if(!isIOBufferAllocated)
      AllocateIOBuffers();

  if (bFlush) {
    FlushEncoder();
    return NV_ENC_SUCCESS;
  }

  assert(pEncodeFrame);

  EncodeBuffer *pEncodeBuffer = m_EncodeBufferQueue.GetAvailable();
  if (!pEncodeBuffer) {
    pEncodeBuffer = m_EncodeBufferQueue.GetPending();
    api().ProcessOutput(configuration.fOutput, pEncodeBuffer);
    // UnMap the input buffer after frame done
    if (pEncodeBuffer->stInputBfr.hInputSurface) {
      nvStatus = api().NvEncUnmapInputResource(pEncodeBuffer->stInputBfr.hInputSurface);
      pEncodeBuffer->stInputBfr.hInputSurface = NULL;
    }
    pEncodeBuffer = m_EncodeBufferQueue.GetAvailable();
  }

  // encode width and height
  unsigned int dwWidth = pEncodeBuffer->stInputBfr.dwWidth;
  unsigned int dwHeight = pEncodeBuffer->stInputBfr.dwHeight;

  // Here we copy from Host to Device Memory (CUDA)
  cuvidCtxLock(lock.get(), 0);
  assert(pEncodeFrame->width == dwWidth && pEncodeFrame->height == dwHeight);

  CUDA_MEMCPY2D memcpy2D = {0};
  memcpy2D.srcMemoryType = CU_MEMORYTYPE_DEVICE;
  memcpy2D.srcDevice = pEncodeFrame->dptr;
  memcpy2D.srcPitch = pEncodeFrame->pitch;
  memcpy2D.dstMemoryType = CU_MEMORYTYPE_DEVICE;
  memcpy2D.dstDevice = (CUdeviceptr)pEncodeBuffer->stInputBfr.pNV12devPtr;
  memcpy2D.dstPitch = pEncodeBuffer->stInputBfr.uNV12Stride;
  memcpy2D.WidthInBytes = dwWidth;
  memcpy2D.Height = dwHeight * 3 / 2;
  __cu(cuMemcpy2D(&memcpy2D));

  cuvidCtxUnlock(lock.get(), 0);

  nvStatus = api().NvEncMapInputResource(pEncodeBuffer->stInputBfr.nvRegisteredResource,
                                                   &pEncodeBuffer->stInputBfr.hInputSurface);
  if (nvStatus != NV_ENC_SUCCESS) {
    PRINTERR("Failed to Map input buffer %p\n", pEncodeBuffer->stInputBfr.hInputSurface);
    return nvStatus;
  }

  api().NvEncEncodeFrame(pEncodeBuffer, NULL, picType);
  //api.NvEncEncodeFrame(pEncodeBuffer, NULL, pEncodeFrame->width, pEncodeFrame->height, picType);
  m_iEncodedFrames++;

  return NV_ENC_SUCCESS;
}
