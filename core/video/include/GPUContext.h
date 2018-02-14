#ifndef LIGHTDB_GPUCONTEXT_H
#define LIGHTDB_GPUCONTEXT_H

#include "errors.h"
#include <glog/logging.h>
#include <stdexcept>
#include <dynlink_nvcuvid.h>

class GPUContext {
public:
    explicit GPUContext(const unsigned int deviceId): device(0) {
        CUresult result;

        if(!ensureInitialized())
            throw GpuRuntimeError("GPU context initialization failed");
        else if((result = cuCtxGetCurrent(&context)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuCtxGetCurrent failed", result);
        else if(context != nullptr)
            ;
        else if((result = cuDeviceGet(&device, deviceId)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError(std::string("Call to cuDeviceGet failed for device ") + std::to_string(deviceId),
                                      result);
        else if((result = cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError(std::string("Call to cuCtxCreate failed for device ") + std::to_string(deviceId),
                                      result);
    }
    ~GPUContext() {
        CUresult result;

        if((result = cuCtxDestroy(context)) != CUDA_SUCCESS)
            LOG(ERROR) << "Swallowed failure to destroy CUDA context (CUresult " << result << ") in destructor";
    }

    CUcontext get() { return context; }

    void AttachToThread() const {
        CUresult result;

        if((result = cuCtxSetCurrent(context)) != CUDA_SUCCESS) {
            throw GpuCudaRuntimeError("Call to cuCtxSetCurrent failed", result);
        }
    }

private:
    static bool isInitialized;
    static bool ensureInitialized() {
            CUresult result;

            if(isInitialized)
                return true;
            else if((result = cuInit(0, __CUDA_API_VERSION, nullptr)) != CUDA_SUCCESS)
                throw GpuCudaRuntimeError("Call to cuInit failed", result);
            else if(cuvidInit(0) != CUDA_SUCCESS)
                throw GpuCudaRuntimeError("Call to cuvidInit failed", result);
            else
                return (isInitialized = true);
        }

    CUdevice device;
    CUcontext context = nullptr;
};

#endif //LIGHTDB_GPUCONTEXT_H
