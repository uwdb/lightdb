#include "Frame.h"

std::shared_ptr<CudaFrame> DecodedFrame::cuda() {
    return cuda_ != nullptr
           ? cuda_
           : (cuda_ = std::make_shared<CudaDecodedFrame>(*this));
}

CudaFrame::CudaFrame(const LocalFrame& frame)
    : CudaFrame(static_cast<const Frame&>(frame))
    {
    CUresult status;
    auto params = CUDA_MEMCPY2D {
            .srcXInBytes = 0,
            .srcY = 0,
            .srcMemoryType = CU_MEMORYTYPE_HOST,
            .srcHost = frame.data().data(),
            .srcDevice = 0,
            .srcArray = nullptr,
            .srcPitch = 0,

            .dstXInBytes = 0,
            .dstY = 0,

            .dstMemoryType = CU_MEMORYTYPE_DEVICE,
            .dstHost = nullptr,
            .dstDevice = handle(),
            .dstArray = nullptr,
            .dstPitch = pitch(),

            .WidthInBytes = width(),
            .Height = height() * 3 / 2
    };

    if((status = cuMemcpy2D(&params)) != CUDA_SUCCESS)
        throw GpuCudaRuntimeError("Call to cuMemcpy2D failed", status);
}