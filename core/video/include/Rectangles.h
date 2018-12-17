#ifndef LIGHTDB_RECTANGLES_H
#define LIGHTDB_RECTANGLES_H

#include "Frame.h"
#include "Rectangle.h"
#include "CudaKernel.h"

namespace lightdb::video {

class GPURectangleOverlay {
public:
    class NV12: public CudaKernel {
    public:
        NV12(const GPUContext &context, const std::experimental::filesystem::path &module_path)
                : CudaKernel(context, module_path, module_filename, function_name)
        { }

        CudaFrameReference draw(VideoLock &lock, const CudaFrameReference &input,
                                const std::vector<Rectangle> &boxes, const unsigned int line_width=2u) const {
            auto output = GPUFrameReference::make<CudaFrame>(static_cast<Frame&>(*input));
            auto &cuda = output.downcast<CudaFrame>();

            cuda.copy(lock, *input);

            draw(lock,
                 cuda.handle(), cuda.height(), cuda.width(), cuda.pitch(),
                 boxes.data(), boxes.size(), line_width);

            return output;
        }

        void draw(VideoLock &lock, CUdeviceptr frame,
                  unsigned int height, unsigned int width, unsigned int pitch,
                  const Rectangle *boxes, const size_t box_count,
                  unsigned int line_width=2u) const {
            CUresult result;
            CUdeviceptr device_boxes;

            if((result = cuMemAlloc(&device_boxes, sizeof(Rectangle) * box_count)) != CUDA_SUCCESS)
                throw GpuCudaRuntimeError("cuMemAlloc failure", result);
            else if((result = cuMemcpyHtoD(device_boxes, boxes, sizeof(Rectangle) * box_count)) != CUDA_SUCCESS)
                throw GpuCudaRuntimeError("cuMemCpy failure", result);

            try {
                draw(lock, frame, height, width, pitch,
                     device_boxes, static_cast<unsigned int>(box_count), line_width);
            } catch(errors::_GpuCudaRuntimeError&) {
                if((result = cuMemFree(device_boxes)) != CUDA_SUCCESS)
                    LOG(ERROR) << "Swallowed failed cuMemFree invocation with result " << result;
                throw;
            }

            if((result = cuMemFree(device_boxes)) != CUDA_SUCCESS)
                throw GpuCudaRuntimeError("cuMemFree failure", result);
        }

        void draw(VideoLock &lock, CUdeviceptr frame,
                  unsigned int height, unsigned int width, unsigned int pitch,
                  CUdeviceptr boxes, unsigned int box_count,
                  unsigned int line_width=2u) const {
            dim3 block(32u, 32u, 1u);
            dim3 grid(width / block.x + 1, height / block.y + 1, box_count);

            void *args[] = { &frame,
                             &height, &width,
                             &pitch,
                             &boxes, &box_count, &line_width };

            invoke(lock, block, grid, args);
        }

    private:
        constexpr static const char* function_name = "draw_rectangles";
        constexpr static const char* module_filename = "draw_rectangles.ptx";
    };

    explicit GPURectangleOverlay(const GPUContext &context)
            : GPURectangleOverlay(context, {"modules"})
    { }

    GPURectangleOverlay(const GPUContext &context, const std::experimental::filesystem::path &module_path)
            : nv12_(context, module_path)
    { }

    const NV12 &nv12() const { return nv12_; }

private:
    NV12 nv12_;

};

} // lightdb::video

#endif //LIGHTDB_RECTANGLES_H
