#include <GPUContext.h>

CUresult CUDAAPI cuvidInit(unsigned int);

bool GPUContext::initialized_ = false;

bool GPUContext::Initialize() {
    CUresult result;

    if(initialized_)
        return true;
    else if((result = cuInit(0)) != CUDA_SUCCESS)
        throw GpuCudaRuntimeError("Call to cuInit failed", result);
    else if((result = cuvidInit(0)) != CUDA_SUCCESS)
        throw GpuCudaRuntimeError("Call to cuvidInit failed", result);
    else
        return (initialized_ = true);
}
