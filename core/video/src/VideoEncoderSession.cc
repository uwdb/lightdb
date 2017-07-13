#include "VideoEncoderSession.h"

NVENCSTATUS VideoEncoderSession::Encode(EncodeFrameConfig &frame, NV_ENC_PIC_STRUCT type) {
    EncodeBuffer *buffer = GetAvailableBuffer();
    NVENCSTATUS status;

    assert(buffer);
    assert(frame.width == buffer->stInputBfr.dwWidth);
    assert(frame.height == buffer->stInputBfr.dwHeight);

    // Copy frame from Host to Device Memory (CUDA)
    if(cuvidCtxLock(encoder.lock.get(), 0) != CUDA_SUCCESS)
        return NV_ENC_ERR_GENERIC;

    CUDA_MEMCPY2D copy = {};
    copy.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    copy.srcDevice = frame.dptr;
    copy.srcPitch = frame.pitch;
    copy.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    copy.dstDevice = (CUdeviceptr)buffer->stInputBfr.pNV12devPtr;
    copy.dstPitch = buffer->stInputBfr.uNV12Stride;
    copy.WidthInBytes = buffer->stInputBfr.dwWidth;
    copy.Height = buffer->stInputBfr.dwHeight * 3 / 2;

    if (cuMemcpy2D(&copy) != CUDA_SUCCESS)
      return NV_ENC_ERR_GENERIC;
    else if(cuvidCtxUnlock(encoder.lock.get(), 0))
        return NV_ENC_ERR_GENERIC;
    else if((status = encoder.api().NvEncMapInputResource(buffer->stInputBfr.nvRegisteredResource,
                                                        &buffer->stInputBfr.hInputSurface)) != NV_ENC_SUCCESS)
        return status;
    else if((status = encoder.api().NvEncEncodeFrame(buffer, NULL, type)) != NV_ENC_SUCCESS)
        return status;

    frameCount_++;

    return NV_ENC_SUCCESS;
}

NVENCSTATUS VideoEncoderSession::Flush() {
    EncodeBuffer *buffer;
    NVENCSTATUS status;

    if((status = encoder.api().NvEncFlushEncoderQueue(encoder.m_stEOSOutputBfr.hOutputEvent)) != NV_ENC_SUCCESS)
        return status;

    while(CompletePendingBuffer() != nullptr);

    writer.Flush();

    return NV_ENC_SUCCESS;
}

EncodeBuffer *VideoEncoderSession::GetAvailableBuffer() {
    EncodeBuffer *buffer = encoder.m_EncodeBufferQueue.GetAvailable();
    if(buffer == nullptr)
        CompletePendingBuffer();

    return buffer != nullptr ? buffer : encoder.m_EncodeBufferQueue.GetAvailable();
}

EncodeBuffer *VideoEncoderSession::CompletePendingBuffer() {
    EncodeBuffer *buffer = encoder.m_EncodeBufferQueue.GetPending();

    if(buffer != nullptr) {
        writer.WriteFrame(*buffer);

        // Unmap the input buffer now that we're done with the frame
        if (buffer->stInputBfr.hInputSurface) {
            if (encoder.api().NvEncUnmapInputResource(buffer->stInputBfr.hInputSurface) != NV_ENC_SUCCESS)
                return nullptr;
            buffer->stInputBfr.hInputSurface = nullptr;
        }
    }

    return buffer;
}