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

        if ((result = cuvidCtxLockCreate(&lock, context.get())) != CUDA_SUCCESS)
            throw result; // TODO
    }
    ~VideoLock() {
        cuvidCtxLockDestroy(lock);
    }

    CUvideoctxlock get() { return lock; }

private:
    const GPUContext& context;
    CUvideoctxlock lock = nullptr;
};

#endif //VISUALCLOUD_VIDEOLOCK_H
