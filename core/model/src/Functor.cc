#include <mutex>
#include <dynlink_builtin_types.h>
#include "LightField.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
        : network_(load_network("/home/bhaynes/projects/darknet/cfg/yolo.cfg", "/home/bhaynes/projects/darknet/yolo.weights", 0)),
          metadata_(get_metadata("/home/bhaynes/projects/darknet/cfg/coco.data"))
    { }

    ObjectDetect::operator const FrameTransform() const {
        CUresult result;
        auto threshold = 0.5f, hier_thresh=0.5f, nms=.45f;
        auto num = num_boxes(network_);
        //auto im = load_image("/home/bhaynes/projects/darknet/data/dog.jpg", 0, 0, 3);
        //auto im = load_image("/home/bhaynes/projects/darknet/data/dog416.jpg", 416, 416, 3);
        auto *boxes = make_boxes(network_);
        auto **probs = make_probs(network_);

        CUdeviceptr resizedFrame;
        CUdeviceptr rgbFrame;
        size_t rgbPitch = 3840*sizeof(unsigned int);

        assert(system("/home/bhaynes/projects/visualcloud/core/model/src/compile_kernels.sh") == 0);

        //result = cuMemAllocPitch(&rgbFrame, &rgbPitch, 3840, 2048, sizeof(unsigned int));
        result = cuMemAlloc(&rgbFrame, 3840 * 2048 * sizeof(unsigned int));
        if(result != CUDA_SUCCESS) printf("result %d\n", result);

        result = cuMemAlloc(&resizedFrame, 3*416 * 416 * sizeof(float));
        if(result != CUDA_SUCCESS) printf("result %d\n", result);

        //std::shared_ptr<unsigned int[]> rgbHostShared( new unsigned int[3840*2048] );
        //auto *rgbHost = rgbHostShared.get();
        unsigned int *rgbHost;
        cuMemAllocHost((void **)&rgbHost, 3840 * 2048 * sizeof(unsigned int));
        //cuMemHostAlloc

        result = cuCtxSynchronize();
        if(result != CUDA_SUCCESS) printf("cuCtxSynchronize result %d\n", result);

        CUdeviceptr boxDevice, probDevice;
        result = cuMemAlloc(&boxDevice, sizeof(box) * num);
        if(result != CUDA_SUCCESS) printf("cuMemAlloc result %d\n", result);
        result = cuMemAlloc(&probDevice, sizeof(float) * num * metadata_.classes);
        if(result != CUDA_SUCCESS) printf("cuMemAlloc result %d\n", result);

        void *flatprobs;
        void *flatboxes;
        void *hostframe;
        cuMemHostAlloc(&flatprobs, sizeof(float) * num * metadata_.classes, CU_MEMHOSTALLOC_WRITECOMBINED);
        cuMemHostAlloc(&flatboxes, sizeof(box) * num, CU_MEMHOSTALLOC_WRITECOMBINED);
        cuMemHostAlloc(&hostframe, sizeof(float) * 3 *  416 * 416, 0);


        if(module_ == nullptr && (result = cuModuleLoad(&module_, "/home/bhaynes/projects/visualcloud/core/model/src/kernels.cubin")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading module") + std::to_string(result));
        else if(function_ == nullptr && (result = cuModuleGetFunction(&function_, module_, "resize_weighted")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading kernel") + std::to_string(result));

        CUfunction nv12_to_rgb, resize = function_;
        if((result = cuModuleGetFunction(&nv12_to_rgb, module_, "NV12_to_RGB")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading NV12_to_RGB kernel") + std::to_string(result));

        CUfunction draw_detections;
        if((result = cuModuleGetFunction(&draw_detections, module_, "draw_detections")) != CUDA_SUCCESS)
            throw new std::runtime_error(std::string("Failure loading draw_detections kernel") + std::to_string(result));

        //result = cuMemFree(output);
        //result = cuMemFree(rgbFrame);

        return [=](VideoLock &lock, Frame& frame) mutable -> Frame& {
            static unsigned int id = 0;

            std::scoped_lock{lock};

            if(id++ % 15 == 0)
            {
            dim3 rgbBlockDims(32, 8);
            dim3 rgbGridDims(std::ceil(float(frame.width()) / (2 * rgbBlockDims.x)), std::ceil(float(frame.height()) / rgbBlockDims.y));
            CUdeviceptr input = frame.handle(); //, output = frame.handle();
            auto height = frame.height(), width = frame.width(), input_pitch = frame.pitch();
            unsigned int output_height = 416, output_width = 416;
            float fx = 416.0f/width, fy = 416.0f/height;

            void *rgb_arguments[6] = {&input, &input_pitch, &rgbFrame, &rgbPitch, &width, &height};
            result = cuLaunchKernel(nv12_to_rgb, rgbGridDims.x, rgbGridDims.y, rgbGridDims.z, rgbBlockDims.x, rgbBlockDims.y, rgbBlockDims.z,
                                    0, nullptr, rgb_arguments, nullptr);
            if(result != CUDA_SUCCESS) printf("result %d\n", result);
            result = cuCtxSynchronize();
            if(result != CUDA_SUCCESS) printf("cuCtxSynchronize result %d\n", result);

            result = cuMemcpyDtoH(rgbHost, rgbFrame, 3840*2048*sizeof(unsigned int));
            if(result != CUDA_SUCCESS) printf("cuMemcpy2D result %d\n", result);
            //stbi_write_bmp("foo.bmp", 3840, 2048, 4, rgbHost);

            //dim3 resizeBlockDims(1,1,1);
            //dim3 resizeGridDims((unsigned int) std::ceil(output_width / resizeBlockDims.x), (unsigned int) std::ceil(output_height / resizeBlockDims.y), 1 );
            dim3 resizeBlockDims(32,32,1);
            dim3 resizeGridDims(1+416/32, 1+416/32, 1 );
            void *resize_arguments[8] = {&rgbFrame, &resizedFrame, &width, &height, &output_width, &output_height, &fx, &fy};
            //void *resize_arguments[8] = {&input, &output, &height, &width, &output_width, &output_height, &fx, &fy};
            result = cuLaunchKernel(resize, resizeGridDims.x, resizeGridDims.y, resizeGridDims.z, resizeBlockDims.x, resizeBlockDims.y, resizeBlockDims.z,
                                   0, nullptr, resize_arguments, nullptr);
            if(result != CUDA_SUCCESS) printf("result %d\n", result);
            result = cuCtxSynchronize();
            if(result != CUDA_SUCCESS) printf("cuCtxSynchronize result %d\n", result);

            result = cuMemcpyDtoH(hostframe, resizedFrame, 3*416*416*sizeof(float));

            //stbi_write_bmp("bar.bmp", 416, 416, 4, hostframe);
            if(result != CUDA_SUCCESS) printf("result %d\n", result);
            image hostimage{416, 416, 3, (float*)hostframe};
                static int index = 0;
            //save_image_png(hostimage, (std::string("frame") + std::to_string(index++)).c_str());

            network_detect(network_, hostimage, threshold, hier_thresh, nms, boxes, probs);

                memcpy(flatboxes, boxes, sizeof(box) * num);
                //float flatprobs[num * metadata_.classes];
                void *current = flatprobs;
                for(int i = 0; i < num; i++)
                {
                    size_t s = sizeof(float) * metadata_.classes;
                    memcpy(current, probs[i], s);
                    current += s;
                }
            cuMemcpyHtoD(boxDevice, flatboxes, sizeof(box) * num);
            cuMemcpyHtoD(probDevice, flatprobs, sizeof(float) * num * metadata_.classes);


            dim3 detectBlockDims(32,32,1);
            dim3 detectGridDims(1+metadata_.classes/32, 1+num/32, 1 );
            void *detect_arguments[10] = {&input, &input_pitch, &width, &height, &probDevice, const_cast<int*>(&metadata_.classes), &boxDevice, &num, &fx, &fy};
            result = cuLaunchKernel(draw_detections, detectGridDims.x, detectGridDims.y, detectGridDims.z, detectBlockDims.x, detectBlockDims.y, detectBlockDims.z,
                                    0, nullptr, detect_arguments, nullptr);
            if(result != CUDA_SUCCESS) printf("result %d\n", result);
            result = cuCtxSynchronize();
            if(result != CUDA_SUCCESS)
                printf("cuCtxSynchronize result %d\n", result);

                /*for(auto i = 0u; i < num; i++)
                    for(auto j = 0u; j < metadata_.classes; j++)
                        if(probs[i][j] - ((float*)flatprobs)[i * metadata_.classes + j] > 0.0001)
                            printf("%d %d\n", i, j);*/

                /*for(auto i = 0u; i < num; i++)
                    for(auto j = 0u; j < metadata_.classes; j++)
                        if(probs[i][j] > 0.001)
                            printf("%d %d %s (%.2f): %X %f\n", i, j, metadata_.names[j], boxes[i].x, *(int *)&probs[i][j], probs[i][j]);*/
                //printf("---\n");
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