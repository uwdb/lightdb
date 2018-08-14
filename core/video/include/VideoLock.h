#ifndef LIGHTDB_VIDEOLOCK_H
#define LIGHTDB_VIDEOLOCK_H

#include "GPUContext.h"

class VideoLock
{
public:
    explicit VideoLock(GPUContext& context) : context_(context) { // TODO shared pointer for context
        CUresult result;

        if ((result = cuvidCtxLockCreate(&lock_, context.get())) != CUDA_SUCCESS) {
            throw GpuCudaRuntimeError("Call to cuvidCtxLockCreate failed", result);
        }
    }
    ~VideoLock() {
        if(lock_ != nullptr)
            cuvidCtxLockDestroy(lock_);
    }

    VideoLock(const VideoLock&) = delete;
    VideoLock(VideoLock&& other) noexcept
            : context_(other.context_), lock_(other.lock_)
    { other.lock_ = nullptr; }

    const GPUContext &context() const { return context_; }
    CUvideoctxlock get() const { return lock_; }

    void lock() {
        CUresult result;
        if ((result = cuvidCtxLock(get(), 0)) != CUDA_SUCCESS) {
            throw GpuCudaRuntimeError("Call to cuvidCtxLock failed", result);
        }
    }

    void unlock() {
        CUresult result;
        if ((result = cuvidCtxUnlock(get(), 0)) != CUDA_SUCCESS) {
            throw GpuCudaRuntimeError("Call to cuvidCtxUnlock failed", result);
        }
    }

private:
    const GPUContext& context_;
    CUvideoctxlock lock_ = nullptr;
};

#endif //LIGHTDB_VIDEOLOCK_H
