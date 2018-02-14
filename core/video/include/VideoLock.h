#ifndef LIGHTDB_VIDEOLOCK_H
#define LIGHTDB_VIDEOLOCK_H

#include <dynlink_cuda.h>
#include <dynlink_cuviddec.h>
#include <cstdio>
#include "GPUContext.h"

class VideoLock
{
public:
    VideoLock(GPUContext& context) : context_(context) { // TODO shared pointer for context
        CUresult result;

        if ((result = cuvidCtxLockCreate(&lock_, context.get())) != CUDA_SUCCESS) {
            throw GpuCudaRuntimeError("Call to cuvidCtxLockCreate failed", result);
        }
    }
    ~VideoLock() {
        cuvidCtxLockDestroy(lock_);
    }

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
