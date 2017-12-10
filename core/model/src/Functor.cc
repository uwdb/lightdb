#include <mutex>
#include <dynlink_builtin_types.h>
#include "LightField.h"

namespace visualcloud {
    const YUVColorSpace::Color Greyscale::operator()(const LightField<YUVColorSpace> &field,
                                                     const Point6D &point) const {
        return YUVColor{field.value(point).y(), 0, 0};
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

    const YUVColorSpace::Color GaussianBlur::operator()(const LightField<YUVColorSpace> &field,
                                                        const Point6D &point) const {
        throw new std::runtime_error("Not implemented");
    }

    GaussianBlur::operator const FrameTransform() const {
        CUresult result;
        //TODO hardcoded path; cannot call in constructor
        if(module_ == nullptr && (result = cuModuleLoad(&module_, "/home/bhaynes/projects/visualcloud/core/model/src/blur.cubin")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading module") + std::to_string(result));
        else if(function_ == nullptr && (result = cuModuleGetFunction(&function_, module_, "blur")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading kernel") + std::to_string(result));

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

    const YUVColorSpace::Color Identity::operator()(const LightField<YUVColorSpace> &field,
                                                     const Point6D &point) const {
        return field.value(point);
    }

    Identity::operator const FrameTransform() const {
        return [](VideoLock&, Frame& frame) -> Frame& {
            return frame;
        };
    };
}; // namespace visualcloud