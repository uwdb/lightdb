#include <mutex>
#include "VideoEncoderSession.h"


NVENCSTATUS VideoEncoderSession::Encode(Frame &frame) {
    auto &buffer = GetAvailableBuffer();
    NVENCSTATUS status;

    if(frame.width() != buffer.stInputBfr.dwWidth ||
            frame.height() != buffer.stInputBfr.dwHeight) {
        status = NV_ENC_ERR_INVALID_PARAM;
    } else {
        buffer.copy(encoder().lock, {
             .srcXInBytes = 0,
             .srcY = 0,
             .srcMemoryType = CU_MEMORYTYPE_DEVICE,
             .srcHost = nullptr,
             .srcDevice = frame.handle(),
             .srcArray = nullptr,
             .srcPitch = frame.pitch(),

             .dstXInBytes = 0,
             .dstY = 0,

             .dstMemoryType = CU_MEMORYTYPE_DEVICE,
             .dstHost = nullptr,
             .dstDevice = static_cast<CUdeviceptr>(buffer.stInputBfr.pNV12devPtr),
             .dstArray = nullptr,
             .dstPitch = buffer.stInputBfr.uNV12Stride,

             .WidthInBytes = buffer.stInputBfr.dwWidth,
             .Height = buffer.stInputBfr.dwHeight * 3 / 2
        });
        //NV_ENC_PIC_PARAMS::encodePicFlags = NV_ENC_PIC_FLAG_OUTPUT_SPSPPS | NV_ENC_PIC_FLAG_FORCEIDR;

        std::scoped_lock{buffer};
        if (//(status = encoder_.api().NvEncMapInputResource(buffer.stInputBfr.nvRegisteredResource,
              //                                            &buffer.stInputBfr.hInputSurface)) == NV_ENC_SUCCESS &&
            (status = encoder_.api().NvEncEncodeFrame(&buffer, nullptr, frame.type(), frameCount() == 0)) == NV_ENC_SUCCESS) {
            frameCount_++;
            status = NV_ENC_SUCCESS;
        }
    }

    return status;
}

NVENCSTATUS VideoEncoderSession::Encode(Frame& frame, size_t top, size_t left) {
    auto &buffer = GetAvailableBuffer();
    NVENCSTATUS status;

    assert(buffer.stInputBfr.bufferFmt == NV_ENC_BUFFER_FORMAT_NV12_PL);

    if(frame.width() - left < buffer.stInputBfr.dwWidth ||
       frame.height() < buffer.stInputBfr.dwHeight) {
        status = NV_ENC_ERR_INVALID_PARAM;
    } else {
        CUDA_MEMCPY2D lumaPlaneParameters = {
                srcXInBytes:   left,
                srcY:          top,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       nullptr,
                srcDevice:     frame.handle(),
                srcArray:      nullptr,
                srcPitch:      frame.pitch(),

                dstXInBytes:   0,
                dstY:          0,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       nullptr,
                dstDevice:     static_cast<CUdeviceptr>(buffer.stInputBfr.pNV12devPtr),
                dstArray:      nullptr,
                dstPitch:      buffer.stInputBfr.uNV12Stride,

                WidthInBytes:  buffer.stInputBfr.dwWidth,
                Height:        buffer.stInputBfr.dwHeight,
        };

        CUDA_MEMCPY2D chromaPlaneParameters = {
                srcXInBytes:   left,
                srcY:          (frame.height() + top) / 2,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       nullptr,
                srcDevice:     frame.handle(),
                srcArray:      nullptr,
                srcPitch:      frame.pitch(),

                dstXInBytes:   0,
                dstY:          buffer.stInputBfr.dwHeight,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       nullptr,
                dstDevice:     static_cast<CUdeviceptr>(buffer.stInputBfr.pNV12devPtr),
                dstArray:      nullptr,
                dstPitch:      buffer.stInputBfr.uNV12Stride,

                WidthInBytes:  buffer.stInputBfr.dwWidth,
                Height:        buffer.stInputBfr.dwHeight / 2
        };

        buffer.copy(encoder().lock, {lumaPlaneParameters, chromaPlaneParameters});

        std::scoped_lock{buffer};
        if (//(status = encoder_.api().NvEncMapInputResource(buffer.stInputBfr.nvRegisteredResource,
              //                                            &buffer.stInputBfr.hInputSurface)) == NV_ENC_SUCCESS &&
            (status = encoder_.api().NvEncEncodeFrame(&buffer, nullptr, frame.type(), frameCount() == 0)) == NV_ENC_SUCCESS) {
            frameCount_++;
            status = NV_ENC_SUCCESS;
        }
    }

    return status;
}


NVENCSTATUS VideoEncoderSession::Flush() {
    NVENCSTATUS status;

    while(CompletePendingBuffer().has_value())
        std::this_thread::yield();

    if((status = encoder_.api().NvEncFlushEncoderQueue(nullptr)) != NV_ENC_SUCCESS)
        return status;

    writer.Flush();

    return NV_ENC_SUCCESS;
}

EncodeBuffer &VideoEncoderSession::GetAvailableBuffer() {
    std::optional<std::shared_ptr<EncodeBuffer>> buffer = queue.GetAvailable();
    if(!buffer.has_value()) {
        CompletePendingBuffer();
        return *queue.GetAvailable().value();
    } else
        return *buffer.value();
}

std::optional<std::shared_ptr<EncodeBuffer>> VideoEncoderSession::CompletePendingBuffer() {
    auto buffer = queue.GetPending();


    if(buffer.has_value()) {
        writer.WriteFrame(*buffer.value());

        // Unmap the input buffer now that we're done with the frame
        if (buffer->get()->stInputBfr.hInputSurface) {
//            if (encoder_.api().NvEncUnmapInputResource(buffer->get().stInputBfr.hInputSurface) != NV_ENC_SUCCESS)
  //              return {};
            buffer->get()->stInputBfr.hInputSurface = nullptr;
        }
    }

    return buffer;
}