#include "Functor.h"
#include <dynlink_builtin_types.h>
#include <mutex>

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

using namespace std::chrono;


namespace lightdb {
    const ColorReference Greyscale::operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const {
        throw std::runtime_error("Encode(YUV) and extra yuv bytes");
        //return YUVColor{field.value(point).y(), 0, 0};
    }

/*    const YUVColorSpace::Color Greyscale::operator()(const LightField &field,
                                                     const Point6D &point) const {
        throw std::runtime_error("Encode(YUV) and extra yuv bytes");
        //return YUVColor{field.value(point).y(), 0, 0};
    }*/

    Greyscale::operator const FrameTransform() const {
        return [](VideoLock& lock, const Frame& frame) -> const Frame& {
            std::scoped_lock{lock};

            auto uv_offset = frame.width() * frame.height();
            auto uv_height = frame.height() / 2;

            auto cudaFrame = dynamic_cast<const GPUFrame&>(frame).cuda();
            //CudaDecodedFrame cudaFrame(frame);
            assert(cuMemsetD2D8(cudaFrame->handle() + uv_offset,
                                cudaFrame->pitch(),
                                128,
                                cudaFrame->width(),
                                uv_height) == CUDA_SUCCESS);
            return frame;
        };
    };

    GaussianBlur::GaussianBlur() : module_(nullptr), function_(nullptr) {
    }

    const ColorReference GaussianBlur::operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const {
        throw new std::runtime_error("Not implemented");
    }

/*    const YUVColorSpace::Color GaussianBlur::operator()(const LightField &field,
                                                        const Point6D &point) const {
        throw new std::runtime_error("Not implemented");
    }*/

    GaussianBlur::operator const FrameTransform() const {
        CUresult result;
        //TODO hardcoded path; cannot call in constructor
        if(module_ == nullptr && (result = cuModuleLoad(&module_, "/home/bhaynes/projects/visualcloud/core/model/src/kernels.cubin")) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Failure loading module", result);
        else if(function_ == nullptr && (result = cuModuleGetFunction(&function_, module_, "blur")) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Failure loading kernel blur", result);

        return [this](VideoLock& lock, const Frame& frame) -> const Frame& {
            std::scoped_lock l{lock};
            auto cudaFrame = dynamic_cast<const GPUFrame&>(frame).cuda();
            //CudaDecodedFrame cudaFrame(frame);

            dim3 blockDims(512,1,1);
            dim3 gridDims((unsigned int) std::ceil((double)(frame.width() * frame.height() * 3 / blockDims.x)), 1, 1 );
            CUdeviceptr input = cudaFrame->handle(), output = cudaFrame->handle();
            auto height = frame.height(), width = frame.width();
            void *arguments[4] = {&input, &output, &height, &width};

            cuLaunchKernel(function_, gridDims.x, gridDims.y, gridDims.z, blockDims.x, blockDims.y, blockDims.z,
                           0, nullptr, arguments, nullptr);

            return frame;
        };
    };

    const ColorReference Identity::operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const {
        throw std::runtime_error("Encode(YUV) and extra yuv bytes");
        //return field.value(point);
    }

/*    const YUVColorSpace::Color Identity::operator()(const LightField &field,
                                                     const Point6D &point) const {
        throw std::runtime_error("Encode(YUV) and extra yuv bytes");
        //return field.value(point);
    }*/

    Identity::operator const FrameTransform() const {
        return [](VideoLock&, const Frame& frame) -> const Frame& {
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
            auto cudaFrame0 = dynamic_cast<const GPUFrame&>(frames[0]).cuda();
            auto cudaFrame1 = dynamic_cast<const GPUFrame&>(frames[1]).cuda();
            //CudaDecodedFrame cudaFrame0(frames[0]);
            //CudaDecodedFrame cudaFrame1(frames[1]);

            assert(frames.size() == 2);

            dim3 blockDims(512,1,1);
            dim3 gridDims((unsigned int) std::ceil((double)(frames[0].width() * frames[0].height() * 3 / blockDims.x)), 1, 1 );
            CUdeviceptr left = cudaFrame0->handle(), right = cudaFrame1->handle(), output = cudaFrame0->handle();
            auto height = frames[0].height(), width = frames[0].width();
            auto transparent = transparent_;
            void *arguments[6] = {&left, &right, &output, &height, &width, &transparent};

            cuLaunchKernel(function_, gridDims.x, gridDims.y, gridDims.z, blockDims.x, blockDims.y, blockDims.z,
                           0, nullptr, arguments, nullptr);

            return frames[0];
        };
    };
}; // namespace lightdb