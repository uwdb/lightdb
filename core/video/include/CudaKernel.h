#ifndef LIGHTDB_CUDAKERNEL_H
#define LIGHTDB_CUDAKERNEL_H

#include "VideoLock.h"
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <experimental/filesystem>
#include <mutex>

namespace lightdb::video {

class CudaKernel {
public:
    CudaKernel(const CudaKernel&) = delete;

protected:
    CudaKernel(const GPUContext &context, const std::experimental::filesystem::path &module_path,
               const char* module_filename, const char* function_name)
            : module_(nullptr),
              function_(nullptr),
              owned_(true) {
        CUresult result;

        if((result = cuModuleLoad(&module_, std::experimental::filesystem::absolute(module_path / module_filename).c_str())) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Could not load module", result);
        else if((result = cuModuleGetFunction(&function_, module_, function_name)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Could not load function", result);
    }

    CudaKernel(CudaKernel&& other) noexcept
            : module_(other.module_),
              function_(other.function_),
              owned_(true)
    { other.owned_ = false; }

    virtual ~CudaKernel() {
        CUresult result;

        if(owned_ && module_ != nullptr && (result = cuModuleUnload(module_)) != CUDA_SUCCESS)
            LOG(WARNING) << "Swallowed attempt to unload module: " << result;
    }

    void invoke(VideoLock &lock, const dim3 &block, const dim3 &grid, void *args[]) const {
        std::lock_guard mlock(lock);

        CUresult result;

        if((result = cuLaunchKernel(function_, grid.x, grid.y, grid.z,
                                    block.x, block.y, block.z,
                                    0,
                                    nullptr, args, nullptr)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Kernel failed", result);
        else if((result = cuStreamQuery(nullptr)) != CUDA_SUCCESS && result != CUDA_ERROR_NOT_READY)
            throw GpuCudaRuntimeError("Kernel cuStreamQuery failed", result);
    }

protected:
    CUmodule module() const { return module_; }
    bool owned() const { return owned_; }

private:
    CUmodule module_;
    CUfunction function_;
    bool owned_;
};

} // lightdb::video

#endif //LIGHTDB_CUDAKERNEL_H
