#include "VideoEncoder.h"
#include "dynlink_cuda.h" // <cuda.h>

#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024

VideoEncoder::VideoEncoder(CUvideoctxlock ctxLock) {
  m_pNvHWEncoder = new CNvHWEncoder;
  m_ctxLock = ctxLock;

  m_uEncodeBufferCount = 0;
  m_iEncodedFrames = 0;
  memset(&m_stEncoderInput, 0, sizeof(m_stEncoderInput));
  memset(&m_stEOSOutputBfr, 0, sizeof(m_stEOSOutputBfr));
  memset(&m_stEncodeBuffer, 0, sizeof(m_stEncodeBuffer));
}

VideoEncoder::~VideoEncoder(void) {
  // clean up encode API resources here
  if (m_pNvHWEncoder) {
    delete m_pNvHWEncoder;
    m_pNvHWEncoder = NULL;
  }
}

NVENCSTATUS VideoEncoder::AllocateIOBuffers(EncodeConfig *pEncodeConfig) {
  NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

  m_uEncodeBufferCount = pEncodeConfig->numB + 4;

  uint32_t uInputWidth = pEncodeConfig->width;
  uint32_t uInputHeight = pEncodeConfig->height;
  m_EncodeBufferQueue.Initialize(m_stEncodeBuffer, m_uEncodeBufferCount);
  m_stEncoderInput.enableAsyncMode = pEncodeConfig->enableAsyncMode;

  // Allocate input buffer
  for (uint32_t i = 0; i < m_uEncodeBufferCount; i++) {
    __cu(cuvidCtxLock(m_ctxLock, 0));
    __cu(cuMemAllocPitch(&m_stEncodeBuffer[i].stInputBfr.pNV12devPtr,
                         (size_t *)&m_stEncodeBuffer[i].stInputBfr.uNV12Stride,
                         uInputWidth, uInputHeight * 3 / 2, 16));
    __cu(cuvidCtxUnlock(m_ctxLock, 0));

    nvStatus = m_pNvHWEncoder->NvEncRegisterResource(
        NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR,
        (void *)m_stEncodeBuffer[i].stInputBfr.pNV12devPtr, uInputWidth,
        uInputHeight, m_stEncodeBuffer[i].stInputBfr.uNV12Stride,
        &m_stEncodeBuffer[i].stInputBfr.nvRegisteredResource);

    if (nvStatus != NV_ENC_SUCCESS)
      return nvStatus;

    m_stEncodeBuffer[i].stInputBfr.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12_PL;
    m_stEncodeBuffer[i].stInputBfr.dwWidth = uInputWidth;
    m_stEncodeBuffer[i].stInputBfr.dwHeight = uInputHeight;

    nvStatus = m_pNvHWEncoder->NvEncCreateBitstreamBuffer(
        BITSTREAM_BUFFER_SIZE,
        &m_stEncodeBuffer[i].stOutputBfr.hBitstreamBuffer);
    if (nvStatus != NV_ENC_SUCCESS)
      return nvStatus;
    m_stEncodeBuffer[i].stOutputBfr.dwBitstreamBufferSize =
        BITSTREAM_BUFFER_SIZE;

    if (m_stEncoderInput.enableAsyncMode) {
      nvStatus = m_pNvHWEncoder->NvEncRegisterAsyncEvent(
          &m_stEncodeBuffer[i].stOutputBfr.hOutputEvent);
      if (nvStatus != NV_ENC_SUCCESS)
        return nvStatus;
      m_stEncodeBuffer[i].stOutputBfr.bWaitOnEvent = true;
    } else
      m_stEncodeBuffer[i].stOutputBfr.hOutputEvent = NULL;
  }

  m_stEOSOutputBfr.bEOSFlag = TRUE;
  if (m_stEncoderInput.enableAsyncMode) {
    nvStatus =
        m_pNvHWEncoder->NvEncRegisterAsyncEvent(&m_stEOSOutputBfr.hOutputEvent);
    if (nvStatus != NV_ENC_SUCCESS)
      return nvStatus;
  } else
    m_stEOSOutputBfr.hOutputEvent = NULL;

  return NV_ENC_SUCCESS;
}

NVENCSTATUS VideoEncoder::ReleaseIOBuffers() {
  for (uint32_t i = 0; i < m_uEncodeBufferCount; i++) {
    __cu(cuvidCtxLock(m_ctxLock, 0));
    cuMemFree(m_stEncodeBuffer[i].stInputBfr.pNV12devPtr);
    __cu(cuvidCtxUnlock(m_ctxLock, 0));

    m_pNvHWEncoder->NvEncDestroyBitstreamBuffer(
        m_stEncodeBuffer[i].stOutputBfr.hBitstreamBuffer);
    m_stEncodeBuffer[i].stOutputBfr.hBitstreamBuffer = NULL;

    if (m_stEncoderInput.enableAsyncMode) {
      m_pNvHWEncoder->NvEncUnregisterAsyncEvent(
          m_stEncodeBuffer[i].stOutputBfr.hOutputEvent);
      nvCloseFile(m_stEncodeBuffer[i].stOutputBfr.hOutputEvent);
      m_stEncodeBuffer[i].stOutputBfr.hOutputEvent = NULL;
    }
  }

  if (m_stEOSOutputBfr.hOutputEvent) {
    if (m_stEncoderInput.enableAsyncMode) {
      m_pNvHWEncoder->NvEncUnregisterAsyncEvent(m_stEOSOutputBfr.hOutputEvent);
      nvCloseFile(m_stEOSOutputBfr.hOutputEvent);
      m_stEOSOutputBfr.hOutputEvent = NULL;
    }
  }

  return NV_ENC_SUCCESS;
}

NVENCSTATUS VideoEncoder::FlushEncoder() {
  NVENCSTATUS nvStatus =
      m_pNvHWEncoder->NvEncFlushEncoderQueue(m_stEOSOutputBfr.hOutputEvent);
  if (nvStatus != NV_ENC_SUCCESS) {
    assert(0);
    return nvStatus;
  }

  EncodeBuffer *pEncodeBuffer = m_EncodeBufferQueue.GetPending();
  while (pEncodeBuffer) {
    m_pNvHWEncoder->ProcessOutput(pEncodeBuffer);
    pEncodeBuffer = m_EncodeBufferQueue.GetPending();
    // UnMap the input buffer after frame is done
    if (pEncodeBuffer && pEncodeBuffer->stInputBfr.hInputSurface) {
      nvStatus = m_pNvHWEncoder->NvEncUnmapInputResource(
          pEncodeBuffer->stInputBfr.hInputSurface);
      pEncodeBuffer->stInputBfr.hInputSurface = NULL;
    }
  }
#if defined(NV_WINDOWS)
  if (m_stEncoderInput.enableAsyncMode) {
    if (WaitForSingleObject(m_stEOSOutputBfr.hOutputEvent, 500) !=
        WAIT_OBJECT_0) {
      assert(0);
      nvStatus = NV_ENC_ERR_GENERIC;
    }
  }
#endif
  return nvStatus;
}

NVENCSTATUS VideoEncoder::Deinitialize() {
  NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

  ReleaseIOBuffers();

  nvStatus = m_pNvHWEncoder->NvEncDestroyEncoder();
  if (nvStatus != NV_ENC_SUCCESS) {
    assert(0);
  }

  return NV_ENC_SUCCESS;
}

NVENCSTATUS VideoEncoder::EncodeFrame(EncodeFrameConfig *pEncodeFrame,
                                      NV_ENC_PIC_STRUCT picType, bool bFlush) {
  NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

  if (bFlush) {
    FlushEncoder();
    return NV_ENC_SUCCESS;
  }

  assert(pEncodeFrame);

  EncodeBuffer *pEncodeBuffer = m_EncodeBufferQueue.GetAvailable();
  if (!pEncodeBuffer) {
    pEncodeBuffer = m_EncodeBufferQueue.GetPending();
    m_pNvHWEncoder->ProcessOutput(pEncodeBuffer);
    // UnMap the input buffer after frame done
    if (pEncodeBuffer->stInputBfr.hInputSurface) {
      nvStatus = m_pNvHWEncoder->NvEncUnmapInputResource(
          pEncodeBuffer->stInputBfr.hInputSurface);
      pEncodeBuffer->stInputBfr.hInputSurface = NULL;
    }
    pEncodeBuffer = m_EncodeBufferQueue.GetAvailable();
  }

  // encode width and height
  unsigned int dwWidth = pEncodeBuffer->stInputBfr.dwWidth;
  unsigned int dwHeight = pEncodeBuffer->stInputBfr.dwHeight;

  // Here we copy from Host to Device Memory (CUDA)
  cuvidCtxLock(m_ctxLock, 0);
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

  cuvidCtxUnlock(m_ctxLock, 0);

  nvStatus = m_pNvHWEncoder->NvEncMapInputResource(
      pEncodeBuffer->stInputBfr.nvRegisteredResource,
      &pEncodeBuffer->stInputBfr.hInputSurface);
  if (nvStatus != NV_ENC_SUCCESS) {
    PRINTERR("Failed to Map input buffer %p\n",
             pEncodeBuffer->stInputBfr.hInputSurface);
    return nvStatus;
  }

  m_pNvHWEncoder->NvEncEncodeFrame(pEncodeBuffer, NULL, pEncodeFrame->width,
                                   pEncodeFrame->height, picType);
  m_iEncodedFrames++;

  return NV_ENC_SUCCESS;
}
