#ifndef LIGHTDB_ENCODEBUFFER_H
#define LIGHTDB_ENCODEBUFFER_H

#include "Frame.h"
#include "Configuration.h"
#include "GPUContext.h"
#include "VideoLock.h"
#include "VideoEncoder.h"
#include <mutex>

struct EncodeInputBuffer
{
    unsigned int      width;
    unsigned int      height;
#if defined (NV_WINDOWS)
    IDirect3DSurface9 *surface;
#endif
    CUdeviceptr       device;
    uint32_t          stride;
    void*             registered_resource;
    NV_ENC_INPUT_PTR  input_surface;
    NV_ENC_BUFFER_FORMAT buffer_format;
};

struct EncodeOutputBuffer
{
    unsigned int          bitstreamBufferSize;
    NV_ENC_OUTPUT_PTR     bitstreamBuffer;
    HANDLE                outputEvent;
    bool                  waitOnEvent;
    bool                  EOSFlag;
};

struct EncodeBuffer
{
    EncodeOutputBuffer      output_buffer;
    EncodeInputBuffer       input_buffer;
    VideoEncoder&           encoder;
    const size_t            size;

    //TODO why is this deleted?
    EncodeBuffer(const EncodeBuffer&& other) = delete;

    EncodeBuffer(const EncodeBuffer& other)
            : EncodeBuffer(other.encoder, other.input_buffer.buffer_format, other.size)
    { }

    // TODO is this size reasonable?
    explicit EncodeBuffer(VideoEncoder &encoder,
                          const NV_ENC_BUFFER_FORMAT format=NV_ENC_BUFFER_FORMAT_NV12_PL,
                          const size_t size=4*1024*1024)
            : output_buffer{},
              input_buffer{},
              encoder(encoder), size(size) {
        NVENCSTATUS status;
        CUresult result;

        input_buffer.buffer_format = format;
        input_buffer.width = encoder.configuration().width;
        input_buffer.height = encoder.configuration().height;
        output_buffer.bitstreamBufferSize = size;
        output_buffer.outputEvent = nullptr;

        if(encoder.configuration().height % 2 != 0)
            throw InvalidArgumentError("Buffer height must be even", "configuration.height");
        else if(encoder.configuration().width % 2 != 0)
            throw InvalidArgumentError("Buffer width must be even", "configuration.width");
        else if(!encoder.api().encoderCreated())
            throw GpuRuntimeError("Encoder not created in API");
        else if((result = cuMemAllocPitch(&input_buffer.device,
                                          (size_t*)&input_buffer.stride,
                                          buffer_width(),
                                          buffer_height(),
                                          //encoder.configuration().width,
                                          //encoder.configuration().height * 3 / 2,
                                          16)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuMemAllocPitch failed", result);
        else if((status = encoder.api().NvEncRegisterResource(
                NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR, (void *)input_buffer.device,
                encoder.configuration().width, encoder.configuration().height,
                input_buffer.stride, &input_buffer.registered_resource, format)) != NV_ENC_SUCCESS)
            throw GpuEncodeRuntimeError("Call to api.NvEncRegisterResource failed", status);
        else if((status = encoder.api().NvEncCreateBitstreamBuffer(
                size, &output_buffer.bitstreamBuffer)) != NV_ENC_SUCCESS)
            throw GpuEncodeRuntimeError("Call to api.NvEncCreateBitstreamBuffer failed", status);
    }

    ~EncodeBuffer() {
        NVENCSTATUS status;
        CUresult result;

        if((status = encoder.api().NvEncDestroyBitstreamBuffer(output_buffer.bitstreamBuffer)) != NV_ENC_SUCCESS)
            LOG(ERROR) << "Call to NvEncDestroyBitstreamBuffer failed with status " << status;
        else if((status = encoder.api().NvEncUnregisterResource(input_buffer.registered_resource)) != NV_ENC_SUCCESS)
            LOG(ERROR) << "Call to NvEncUnregisterResource failed with status " << status;
        else if((result = cuMemFree(input_buffer.device)) != CUDA_SUCCESS)
            LOG(ERROR) << "Call to cuMemFree failed with result " << result;
    }

    //TODO These arguments should be GPUFrame
    void copy(VideoLock &lock, Frame &frame) {
        if(frame.width() != input_buffer.width ||
           frame.height() != input_buffer.height) {
            throw InvalidArgumentError("Frame size does not match buffer size", "frame");
        }

        //TODO create new overloads that accept CudaDecodedFrames and avoid unsafe cast
        auto cudaFrame = dynamic_cast<GPUFrame&>(frame).cuda();

        //CudaDecodedFrame cudaFrame(frame);
        copy(lock, {
            .srcXInBytes = 0,
            .srcY = 0,
            .srcMemoryType = CU_MEMORYTYPE_DEVICE,
            .srcHost = nullptr,
            .srcDevice = cudaFrame->handle(),
            .srcArray = nullptr,
            .srcPitch = cudaFrame->pitch(),

            .dstXInBytes = 0,
            .dstY = 0,

            .dstMemoryType = CU_MEMORYTYPE_DEVICE,
            .dstHost = nullptr,
            .dstDevice = static_cast<CUdeviceptr>(input_buffer.device),
            .dstArray = nullptr,
            .dstPitch = input_buffer.stride,

            .WidthInBytes = buffer_width(), //input_buffer.width,
            .Height = buffer_height() //input_buffer.height * 3 / 2
        });
    }

    void copy(VideoLock &lock, Frame &frame,
              size_t frame_top, size_t frame_left,
              size_t buffer_top=0, size_t buffer_left=0) {
        auto cudaFrame = dynamic_cast<GPUFrame&>(frame).cuda();

        if(frame_top == 0 && frame_left == 0 &&
           buffer_top == 0 && buffer_left == 0) {
            copy(lock, frame);
            return;
        }

        //CudaDecodedFrame cudaFrame(frame);
        CUDA_MEMCPY2D lumaPlaneParameters{
                srcXInBytes:   frame_left,
                srcY:          frame_top,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       nullptr,
                srcDevice:     cudaFrame->handle(),
                srcArray:      nullptr,
                srcPitch:      cudaFrame->pitch(),

                dstXInBytes:   buffer_left,
                dstY:          buffer_top,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       nullptr,
                dstDevice:     static_cast<CUdeviceptr>(input_buffer.device),
                dstArray:      nullptr,
                dstPitch:      input_buffer.stride,

                WidthInBytes:  std::min(input_buffer.width - buffer_left, cudaFrame->width() - frame_left),
                Height:        std::min(input_buffer.height - buffer_top, cudaFrame->height() - frame_top) ,
        };

        CUDA_MEMCPY2D chromaPlaneParameters{
                srcXInBytes:   frame_left,
                srcY:          frame.height() + frame_top / 2,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       nullptr,
                srcDevice:     cudaFrame->handle(),
                srcArray:      nullptr,
                srcPitch:      cudaFrame->pitch(),

                dstXInBytes:   buffer_left,
                dstY:          input_buffer.height + buffer_top / 2,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       nullptr,
                dstDevice:     static_cast<CUdeviceptr>(input_buffer.device),
                dstArray:      nullptr,
                dstPitch:      input_buffer.stride,

                WidthInBytes:  std::min(input_buffer.width - buffer_left, cudaFrame->width() - frame_left),
                Height:        std::min(input_buffer.height - buffer_top, cudaFrame->height() - frame_top) / 2
        };

        copy(lock, {lumaPlaneParameters, chromaPlaneParameters});
    }

    void copy(VideoLock &lock, const CUDA_MEMCPY2D &parameters) {
        CUresult result;
        std::scoped_lock l{lock};

        if ((result = cuMemcpy2D(&parameters)) != CUDA_SUCCESS) {
            throw GpuCudaRuntimeError("Call to cuMemcpy2D failed", result);
        }
    }

    void copy(VideoLock &lock, const std::vector<CUDA_MEMCPY2D> &parameters) {
        std::scoped_lock l{lock};
        std::for_each(parameters.begin(), parameters.end(), [](const CUDA_MEMCPY2D &parameters) {
            CUresult result;
            if ((result = cuMemcpy2D(&parameters)) != CUDA_SUCCESS) {
                throw GpuCudaRuntimeError("Call to cuMemcpy2D failed", result);
            }
        });
    }

    void lock() {
        NVENCSTATUS status;
        if((status = encoder.api().NvEncMapInputResource(input_buffer.registered_resource,
                                                         &input_buffer.input_surface)) != NV_ENC_SUCCESS)
            throw GpuEncodeRuntimeError("Call to api.NvEncMapInputResource failed", status);
    }

    void unlock() {
        NVENCSTATUS status;
        if((status = encoder.api().NvEncUnmapInputResource(input_buffer.input_surface)) != NV_ENC_SUCCESS)
            throw GpuEncodeRuntimeError("Call to api.NvEncUnmapInputResource failed", status);
    }

private:
    size_t buffer_height() const {
        switch(input_buffer.buffer_format) {
            case NV_ENC_BUFFER_FORMAT_NV12:
            case NV_ENC_BUFFER_FORMAT_IYUV:
                return encoder.configuration().height * 3 / 2;
            case NV_ENC_BUFFER_FORMAT_YV12:
                return encoder.configuration().height * 2;
            default:
                throw InvalidArgumentError("Unsupported encoded buffer format", "input_buffer.buffer_format");
        }
    }

    size_t buffer_width() const {
        switch(input_buffer.buffer_format) {
            case NV_ENC_BUFFER_FORMAT_NV12:
            case NV_ENC_BUFFER_FORMAT_IYUV:
            case NV_ENC_BUFFER_FORMAT_YV12:
                return encoder.configuration().width;
            default:
                throw InvalidArgumentError("Unsupported encoded buffer format", "input_buffer.buffer_format");
        }
    }
};

struct MotionEstimationBuffer
{
    EncodeOutputBuffer      output_buffer;
    EncodeInputBuffer       input_buffer[2];
    unsigned int            inputFrameIndex;
    unsigned int            referenceFrameIndex;
};

#endif //LIGHTDB_ENCODEBUFFER_H
