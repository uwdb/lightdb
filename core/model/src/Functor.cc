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
        if(module_ == nullptr && (result = cuModuleLoad(&module_, "/home/bhaynes/projects/visualcloud/core/model/src/kernels.cubin")) != CUDA_SUCCESS)
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

    ObjectDetect::ObjectDetect()
        : network_(load_network("/home/bhaynes/projects/darknet/cfg/tiny-yolo.cfg", "/home/bhaynes/projects/darknet/tiny-yolo.weights", 0)),
          metadata_(get_metadata("/home/bhaynes/projects/darknet/cfg/coco.data"))
    { }

    ObjectDetect::operator const FrameTransform() const {
        CUresult result;
        auto threshold = 0.5f, hier_thresh=0.5f, nms=.45f;
        auto num = num_boxes(network_);
        //auto im = load_image("/home/bhaynes/projects/darknet/data/dog.jpg", 0, 0, 3);
        auto im = load_image("/home/bhaynes/projects/darknet/data/dog416.jpg", 416, 416, 3);
        auto *boxes = make_boxes(network_);
        auto **probs = make_probs(network_);

        CUdeviceptr output;
        result = cuMemAlloc(&output, 416 * 416 * sizeof(float));
        if(result != CUDA_SUCCESS) printf("result %d\n", result);

        if(module_ == nullptr && (result = cuModuleLoad(&module_, "/home/bhaynes/projects/visualcloud/core/model/src/kernels.cubin")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading module") + std::to_string(result));
        else if(function_ == nullptr && (result = cuModuleGetFunction(&function_, module_, "resize")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading kernel") + std::to_string(result));

        //result = cuMemFree(output);

        return [=](VideoLock &lock, Frame& frame) mutable -> Frame& {
            static unsigned int id = 0;

            std::scoped_lock{lock};

            if(id++ % 15 == 0)
            {

            dim3 blockDims(512,1,1);
            dim3 gridDims((unsigned int) std::ceil((double)(frame.width() * frame.height() * 3 / blockDims.x)), 1, 1 );
            CUdeviceptr input = frame.handle(); //, output = frame.handle();
            auto height = frame.height(), width = frame.width();
            unsigned int output_height = 416, output_width = 416;
            float fx = 416.0f/width, fy = 416.0f/height;
            void *arguments[8] = {&input, &output, &height, &width, &output_width, &output_height, &fx, &fy};

            result = cuLaunchKernel(function_, gridDims.x, gridDims.y, gridDims.z, blockDims.x, blockDims.y, blockDims.z,
                                   0, nullptr, arguments, nullptr);
            if(result != CUDA_SUCCESS) printf("result %d\n", result);

            float hostframe[416*416];
            result = cuMemcpyDtoH(hostframe, output, 416*416*sizeof(float));
            if(result != CUDA_SUCCESS) printf("result %d\n", result);
            image hostimage{416, 416, 1, hostframe};

                network_detect(network_, hostimage, threshold, hier_thresh, nms, boxes, probs);

                for(auto i = 0u; i < num; i++)
                    for(auto j = 0u; j < metadata_.classes; j++)
                        if(probs[i][j] > 0.001)
                            printf("%s (%d): %0.2f\n", metadata_.names[j], i, probs[i][j]);
            }

            return frame;
        };
    };

    Left::operator const NaryFrameTransform() const {
        return [](VideoLock&, const std::vector<Frame>& frames) -> const Frame& {
            return frames[0];
        };
    };

    Overlay::operator const NaryFrameTransform() const {
        CUresult result;
        //TODO hardcoded path; cannot call in constructor
        if(module_ == nullptr && (result = cuModuleLoad(&module_, "/home/bhaynes/projects/visualcloud/core/model/src/kernels.cubin")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading module") + std::to_string(result));
        else if(function_ == nullptr && (result = cuModuleGetFunction(&function_, module_, "overlay")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading kernel") + std::to_string(result));

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
}; // namespace visualcloud