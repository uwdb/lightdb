#ifndef VISUALCLOUD_GPUCONTEXT_H
#define VISUALCLOUD_GPUCONTEXT_H

#include <dynlink_cuda.h>
#include <cstdio>
#include <dynlink_nvcuvid.h>

class GPUContext {
public:
    GPUContext(unsigned int deviceId) {
        CUresult result;

        if(!ensureInitialized())
            printf("throw");
        else if((result = cuDeviceGet(&device, deviceId)) != CUDA_SUCCESS)
            printf("throw");
        else if((result = cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device)) != CUDA_SUCCESS)
            printf("throw");
        else if((result = cuCtxPopCurrent(&context)) != CUDA_SUCCESS)
            printf("throw");
    }
    ~GPUContext() {
        CUresult result;

        if((result = cuCtxDestroy(context)) != CUDA_SUCCESS)
            printf("log error");
    }

    CUcontext get() { return context; }

private:
    static bool isInitialized;
    static bool ensureInitialized() {
            CUresult result;

            if(isInitialized)
                return true;
            else if((result = cuInit(0, __CUDA_API_VERSION, nullptr)) != CUDA_SUCCESS)
                printf("throw\n");
            else if(cuvidInit(0) != CUDA_SUCCESS)
                printf("throw\n");
            else
                return (isInitialized = true);
        }

    CUdevice device;
    CUcontext context = nullptr;
};

#endif //VISUALCLOUD_GPUCONTEXT_H
