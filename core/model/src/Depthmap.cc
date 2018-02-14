#include "LightField.h"
#include "Functor.h"
#include <dynlink_builtin_types.h>
#include <mutex>

//TODO hardcoded path :(
static const char* kernel_path = "/home/bhaynes/projects/visualcloud/core/model/src/kernels.cubin";
static const char* kernel_name = "blur";

namespace lightdb {

DepthmapCPU::operator const FrameTransform() const {
    return [this](VideoLock &lock, Frame &frame) -> Frame & {
        return frame;
    };
}

DepthmapFPGA::operator const FrameTransform() const {
    return [this](VideoLock &lock, Frame &frame) -> Frame & {
        return frame;
    };
}

DepthmapGPU::operator const FrameTransform() const {
    CUresult result;

    if(module_ == nullptr && (result = cuModuleLoad(&module_, kernel_path)) != CUDA_SUCCESS)
        throw GpuCudaRuntimeError(std::string("Failure loading module ") + kernel_path, result);
    else if(function_ == nullptr && (result = cuModuleGetFunction(&function_, module_, kernel_name)) != CUDA_SUCCESS)
        throw GpuCudaRuntimeError(std::string("Failure loading kernel ") + kernel_name, result);
    else
        return [this](VideoLock& lock, Frame& frame) -> Frame& {
            std::scoped_lock{lock};

            //dim3 blockDims{512,1,1};
            //dim3 gridDims{(unsigned int) std::ceil((double)(frame.width() * frame.height() * 3 / blockDims.x)), 1, 1};
            //CUdeviceptr input = frame.handle(), output = frame.handle();
            //auto height = frame.height(), width = frame.width();
            //void *arguments[4] = {&input, &output, &height, &width};

            //cuLaunchKernel(function_, gridDims.x, gridDims.y, gridDims.z, blockDims.x, blockDims.y, blockDims.z,
            //               0, nullptr, arguments, nullptr);

            return frame;
        };
};

};
