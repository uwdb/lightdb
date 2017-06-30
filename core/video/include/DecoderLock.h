#ifndef VISUALCLOUD_DECODERLOCK_H
#define VISUALCLOUD_DECODERLOCK_H

#include <dynlink_cuda.h>
#include <dynlink_cuviddec.h>
#include <cstdio>

class DecoderLock {
public:
    DecoderLock(CUcontext context) : context(context) { // TODO shared pointer for context
        CUresult result;

        if ((result = cuvidCtxLockCreate(&lock, context)) != CUDA_SUCCESS)
            printf("throw %d\n", result);
    }
    ~DecoderLock() {
        cuvidCtxLockDestroy(lock);
    }

    CUvideoctxlock get() { return lock; }

private:
    const CUcontext context;
    CUvideoctxlock lock = nullptr;
};

#endif //VISUALCLOUD_DECODERLOCK_H
