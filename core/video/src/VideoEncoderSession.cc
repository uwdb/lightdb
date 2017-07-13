#include "VideoEncoderSession.h"

NVENCSTATUS VideoEncoderSession::Encode(EncoderSessionInputFrame &frame, NV_ENC_PIC_STRUCT type) {
    EncodeBuffer &buffer = GetAvailableBuffer();
    NVENCSTATUS status;

    assert(frame.width == buffer.stInputBfr.dwWidth);
    assert(frame.height == buffer.stInputBfr.dwHeight);

    // CUDA copy of frame from host to device memory
    if(cuvidCtxLock(encoder.lock.get(), 0) != CUDA_SUCCESS)
        return NV_ENC_ERR_GENERIC;

    CUDA_MEMCPY2D copy = {};
    copy.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    copy.srcDevice = frame.handle;
    copy.srcPitch = frame.pitch;
    copy.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    copy.dstDevice = (CUdeviceptr)buffer.stInputBfr.pNV12devPtr;
    copy.dstPitch = buffer.stInputBfr.uNV12Stride;
    copy.WidthInBytes = buffer.stInputBfr.dwWidth;
    copy.Height = buffer.stInputBfr.dwHeight * 3 / 2;

    if (cuMemcpy2D(&copy) != CUDA_SUCCESS)
      return NV_ENC_ERR_GENERIC;
    else if(cuvidCtxUnlock(encoder.lock.get(), 0))
        return NV_ENC_ERR_GENERIC;
    else if((status = encoder.api().NvEncMapInputResource(buffer.stInputBfr.nvRegisteredResource,
                                                        &buffer.stInputBfr.hInputSurface)) != NV_ENC_SUCCESS)
        return status;
    else if((status = encoder.api().NvEncEncodeFrame(&buffer, NULL, type)) != NV_ENC_SUCCESS)
        return status;

    frameCount_++;

    return NV_ENC_SUCCESS;
}

NVENCSTATUS VideoEncoderSession::Flush() {
    while(CompletePendingBuffer().has_value());

    writer.Flush();

    return NV_ENC_SUCCESS;
}

EncodeBuffer &VideoEncoderSession::GetAvailableBuffer() {
    auto buffer = queue.GetAvailable();
    if(!buffer.has_value()) {
        CompletePendingBuffer();
        return queue.GetAvailable().value();
    } else
        return buffer.value();
}

std::optional<std::reference_wrapper<EncodeBuffer>> VideoEncoderSession::CompletePendingBuffer() {
    auto buffer = queue.GetPending();

    if(buffer.has_value()) {
        writer.WriteFrame(*buffer);

        // Unmap the input buffer now that we're done with the frame
        if (buffer->get().stInputBfr.hInputSurface) {
            if (encoder.api().NvEncUnmapInputResource(buffer->get().stInputBfr.hInputSurface) != NV_ENC_SUCCESS)
                return {};
            buffer->get().stInputBfr.hInputSurface = nullptr;
        }
    }

    return buffer;
}