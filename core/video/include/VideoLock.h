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
            LOG(ERROR) << "cuvidCtxLockCreate";
            throw std::runtime_error(std::to_string(result) + "VideoLock"); // TODO
        }
    }
    ~VideoLock() {
        cuvidCtxLockDestroy(lock_);
    }

    GPUContext &context() { return context_; }
    CUvideoctxlock get() const { return lock_; }

    void lock() {
        CUresult result;
        if ((result = cuvidCtxLock(get(), 0)) != CUDA_SUCCESS) {
            LOG(ERROR) << "cuvidCtxLock";
            throw std::runtime_error(std::to_string(result) + "lock"); //TODO
        }
    }

    void unlock() {
        CUresult result;
        if ((result = cuvidCtxUnlock(get(), 0)) != CUDA_SUCCESS) {
            LOG(ERROR) << "cuvidCtxUnlock";
            throw std::runtime_error(std::to_string(result) + "unlock"); //TODO
        }
    }

private:
    /*TODO const */ GPUContext& context_;
    CUvideoctxlock lock_ = nullptr;
};

#endif //LIGHTDB_VIDEOLOCK_H
