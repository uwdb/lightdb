#ifndef VISUALCLOUD_VIDEOLOCK_H
#define VISUALCLOUD_VIDEOLOCK_H

#include <dynlink_cuda.h>
#include <dynlink_cuviddec.h>
#include <cstdio>
#include "GPUContext.h"

class VideoLock
{
public:
    VideoLock(GPUContext& context) : context(context) { // TODO shared pointer for context
        CUresult result;

        if ((result = cuvidCtxLockCreate(&lock_, context.get())) != CUDA_SUCCESS)
            throw std::runtime_error(std::to_string(result)); // TODO
    }
    ~VideoLock() {
        cuvidCtxLockDestroy(lock_);
    }

    CUvideoctxlock get() { return lock_; }

    void lock() {
        CUresult result;
        if ((result = cuvidCtxLock(get(), 0)) != CUDA_SUCCESS)
            throw std::runtime_error(std::to_string(result)); //TODO
    }

    void unlock() {
        CUresult result;
        if ((result = cuvidCtxUnlock(get(), 0)) != CUDA_SUCCESS)
            throw std::runtime_error(std::to_string(result)); //TODO
    }

private:
    const GPUContext& context;
    CUvideoctxlock lock_ = nullptr;
};

#endif //VISUALCLOUD_VIDEOLOCK_H
