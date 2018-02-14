#include "Functor.h"
#include <dynlink_builtin_types.h>
#include <mutex>

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std::chrono;


namespace lightdb {
    const YUVColorSpace::Color Greyscale::operator()(const LightField &field,
                                                     const Point6D &point) const {
        throw std::runtime_error("Encode(YUV) and extra yuv bytes");
        //return YUVColor{field.value(point).y(), 0, 0};
    }

    Greyscale::operator const FrameTransform() const {
        return [](VideoLock& lock, Frame& frame) -> Frame& {
            std::scoped_lock{lock};

            auto uv_offset = frame.width() * frame.height();
            auto uv_height = frame.height() / 2;

            assert(cuMemsetD2D8(frame.handle() + uv_offset,
                                frame.pitch(),
                                128,
                                frame.width(),
                                uv_height) == CUDA_SUCCESS);
            return frame;
        };
    };

    GaussianBlur::GaussianBlur() : module_(nullptr), function_(nullptr) {
    }

    const YUVColorSpace::Color GaussianBlur::operator()(const LightField &field,
                                                        const Point6D &point) const {
        throw new std::runtime_error("Not implemented");
    }

    GaussianBlur::operator const FrameTransform() const {
        CUresult result;
        //TODO hardcoded path; cannot call in constructor
        if(module_ == nullptr && (result = cuModuleLoad(&module_, "/home/bhaynes/projects/visualcloud/core/model/src/kernels.cubin")) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Failure loading module", result);
        else if(function_ == nullptr && (result = cuModuleGetFunction(&function_, module_, "blur")) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Failure loading kernel blur", result);

        return [this](VideoLock& lock, Frame& frame) -> Frame& {
            std::scoped_lock{lock};

            dim3 blockDims(512,1,1);
            dim3 gridDims((unsigned int) std::ceil((double)(frame.width() * frame.height() * 3 / blockDims.x)), 1, 1 );
            CUdeviceptr input = frame.handle(), output = frame.handle();
            auto height = frame.height(), width = frame.width();
            void *arguments[4] = {&input, &output, &height, &width};

            cuLaunchKernel(function_, gridDims.x, gridDims.y, gridDims.z, blockDims.x, blockDims.y, blockDims.z,
                           0, nullptr, arguments, nullptr);

            return frame;
        };
    };

    const YUVColorSpace::Color Identity::operator()(const LightField &field,
                                                     const Point6D &point) const {
        throw std::runtime_error("Encode(YUV) and extra yuv bytes");
        //return field.value(point);
    }

    Identity::operator const FrameTransform() const {
        return [](VideoLock&, Frame& frame) -> Frame& {
            return frame;
        };
    };

    Overlay::operator const NaryFrameTransform() const {
        CUresult result;
        //TODO hardcoded path; cannot call in constructor
        if(module_ == nullptr && (result = cuModuleLoad(&module_, "/home/bhaynes/projects/visualcloud/core/model/src/kernels.cubin")) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Failure loading module", result);
        else if(function_ == nullptr && (result = cuModuleGetFunction(&function_, module_, "overlay")) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Failure loading kernel blur", result);

        return [this](VideoLock& lock, const std::vector<Frame>& frames) -> const Frame& {
            std::scoped_lock{lock};

            assert(frames.size() == 2);

            dim3 blockDims(512,1,1);
            dim3 gridDims((unsigned int) std::ceil((double)(frames[0].width() * frames[0].height() * 3 / blockDims.x)), 1, 1 );
            CUdeviceptr left = frames[0].handle(), right = frames[1].handle(), output = frames[0].handle();
            auto height = frames[0].height(), width = frames[0].width();
            auto transparent = transparent_;
            void *arguments[6] = {&left, &right, &output, &height, &width, &transparent};

            cuLaunchKernel(function_, gridDims.x, gridDims.y, gridDims.z, blockDims.x, blockDims.y, blockDims.z,
                           0, nullptr, arguments, nullptr);

            return frames[0];
        };
    };
}; // namespace lightdb