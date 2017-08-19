#include "VideoEncoderSession.h"

NVENCSTATUS VideoEncoderSession::Encode(Frame &frame) {
    auto &buffer = GetAvailableBuffer();
    NVENCSTATUS status;
    CUDA_MEMCPY2D copy = {
        .srcXInBytes = 0,
        .srcY = 0,
        .srcMemoryType = CU_MEMORYTYPE_DEVICE,
        .srcHost = 0,
        .srcDevice = frame.handle(),
        .srcArray = 0,
        .srcPitch = frame.pitch(),

        .dstXInBytes = 0,
        .dstY = 0,

        .dstMemoryType = CU_MEMORYTYPE_DEVICE,
        .dstHost = 0,
        .dstDevice = static_cast<CUdeviceptr>(buffer.stInputBfr.pNV12devPtr),
        .dstArray = 0,
        .dstPitch = buffer.stInputBfr.uNV12Stride,

        .WidthInBytes = buffer.stInputBfr.dwWidth,
        .Height = buffer.stInputBfr.dwHeight * 3 / 2
    };

    if(frame.width() != buffer.stInputBfr.dwWidth ||
            frame.height() != buffer.stInputBfr.dwHeight) {
        status = NV_ENC_ERR_INVALID_PARAM;
    } else if(cuvidCtxLock(encoder.lock.get(), 0) != CUDA_SUCCESS) {
        status = NV_ENC_ERR_GENERIC;
    } else if (cuMemcpy2D(&copy) != CUDA_SUCCESS) {
        status = NV_ENC_ERR_GENERIC;
    } else if (cuvidCtxUnlock(encoder.lock.get(), 0) != CUDA_SUCCESS) {
        status = NV_ENC_ERR_GENERIC;
    } else if ((status = encoder.api().NvEncMapInputResource(buffer.stInputBfr.nvRegisteredResource,
                                                             &buffer.stInputBfr.hInputSurface)) != NV_ENC_SUCCESS) {
        ;
    } else if ((status = encoder.api().NvEncEncodeFrame(&buffer, nullptr, frame.type())) != NV_ENC_SUCCESS) {
        ;
    } else {
        frameCount_++;
        status = NV_ENC_SUCCESS;
    }

    return status;
}

NVENCSTATUS VideoEncoderSession::Flush() {
    while(CompletePendingBuffer().has_value())
        sleep(0);

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