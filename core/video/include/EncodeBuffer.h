#ifndef VISUALCLOUD_ENCODEBUFFER_H
#define VISUALCLOUD_ENCODEBUFFER_H

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
    IDirect3DSurface9 *NV12Surface;
#endif
    CUdeviceptr       NV12devPtr;
    uint32_t          NV12Stride;
    CUdeviceptr       NV12TempdevPtr;
    uint32_t          NV12TempStride;
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

    EncodeBuffer(const EncodeBuffer&& other) = delete;

    EncodeBuffer(const EncodeBuffer& other)
            : EncodeBuffer(other.encoder, other.size)
    { }

    // TODO is this size reasonable?
    EncodeBuffer(VideoEncoder &encoder, size_t size=4*1024*1024)
            : output_buffer{0}, input_buffer{0}, encoder(encoder), size(size) {
        NVENCSTATUS status;

        if(encoder.configuration().height % 2 != 0)
            throw std::runtime_error("Buffer height must be even"); //TODO
        else if(encoder.configuration().width % 2 != 0)
            throw std::runtime_error("Buffer width must be even"); //TODO
        else if(!encoder.api().encoderCreated())
            throw std::runtime_error("encoder not created"); //TODO
        else if(cuMemAllocPitch(&input_buffer.NV12devPtr,
                           (size_t*)&input_buffer.NV12Stride,
                           encoder.configuration().width,
                           encoder.configuration().height * 3 / 2,
                           16) != CUDA_SUCCESS)
            throw std::runtime_error("NV_ENC_ERR_OUT_OF_MEMORY"); //tODO
        else if((status = encoder.api().NvEncRegisterResource(
                NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR, (void *)input_buffer.NV12devPtr,
                encoder.configuration().width, encoder.configuration().height,
                input_buffer.NV12Stride, &input_buffer.registered_resource)) != NV_ENC_SUCCESS)
            throw std::runtime_error(std::to_string(status) + "EncodeBuffer.NvEncRegisterResource"); //TODO
        else if((status = encoder.api().NvEncCreateBitstreamBuffer(
                size, &output_buffer.bitstreamBuffer)) != NV_ENC_SUCCESS)
            throw std::runtime_error("EncodeBuffer.997"); //TODO

        input_buffer.buffer_format = NV_ENC_BUFFER_FORMAT_NV12_PL;
        input_buffer.width = encoder.configuration().width;
        input_buffer.height = encoder.configuration().height;
        output_buffer.bitstreamBufferSize = size;
        output_buffer.outputEvent = nullptr;
    }

    ~EncodeBuffer() {
        NVENCSTATUS status;

        if((status = encoder.api().NvEncDestroyBitstreamBuffer(output_buffer.bitstreamBuffer)) != NV_ENC_SUCCESS)
            printf("log NvEncDestroyBitstreamBuffer\n"); //TODO log
        else if((encoder.api().NvEncUnregisterResource(input_buffer.registered_resource)) != NV_ENC_SUCCESS)
            printf("log NvEncUnregisterResource\n"); //TODO log
        else if(cuMemFree(input_buffer.NV12devPtr) != CUDA_SUCCESS)
            printf("log cuMemFree\n"); //TODO log
    }

    void copy(VideoLock &lock, const Frame &frame) {
        if(frame.width() != input_buffer.width ||
           frame.height() != input_buffer.height) {
            LOG(ERROR) << "frame size does not match buffer size";
            throw std::runtime_error("frame size does not match buffer size"); //TODO
        }

        copy(lock, {
            .srcXInBytes = 0,
            .srcY = 0,
            .srcMemoryType = CU_MEMORYTYPE_DEVICE,
            .srcHost = nullptr,
            .srcDevice = frame.handle(),
            .srcArray = nullptr,
            .srcPitch = frame.pitch(),

            .dstXInBytes = 0,
            .dstY = 0,

            .dstMemoryType = CU_MEMORYTYPE_DEVICE,
            .dstHost = nullptr,
            .dstDevice = static_cast<CUdeviceptr>(input_buffer.NV12devPtr),
            .dstArray = nullptr,
            .dstPitch = input_buffer.NV12Stride,

            .WidthInBytes = input_buffer.width,
            .Height = input_buffer.height * 3 / 2
        });
    }

    void copy(VideoLock &lock, const Frame &frame,
              size_t frame_top, size_t frame_left,
              size_t buffer_top=0, size_t buffer_left=0) {
        CUDA_MEMCPY2D lumaPlaneParameters = {
                srcXInBytes:   frame_left,
                srcY:          frame_top,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       nullptr,
                srcDevice:     frame.handle(),
                srcArray:      nullptr,
                srcPitch:      frame.pitch(),

                dstXInBytes:   buffer_left,
                dstY:          buffer_top,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       nullptr,
                dstDevice:     static_cast<CUdeviceptr>(input_buffer.NV12devPtr),
                dstArray:      nullptr,
                dstPitch:      input_buffer.NV12Stride,

                WidthInBytes:  std::min(input_buffer.width - buffer_left, frame.width() - frame_left),
                Height:        std::min(input_buffer.height - buffer_top, frame.height() - frame_top) ,
        };

        CUDA_MEMCPY2D chromaPlaneParameters = {
                srcXInBytes:   frame_left,
                srcY:          frame.height() + frame_top / 2,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       nullptr,
                srcDevice:     frame.handle(),
                srcArray:      nullptr,
                srcPitch:      frame.pitch(),

                dstXInBytes:   buffer_left,
                dstY:          input_buffer.height + buffer_top / 2,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       nullptr,
                dstDevice:     static_cast<CUdeviceptr>(input_buffer.NV12devPtr),
                dstArray:      nullptr,
                dstPitch:      input_buffer.NV12Stride,

                WidthInBytes:  std::min(input_buffer.width - buffer_left, frame.width() - frame_left),
                Height:        std::min(input_buffer.height - buffer_top, frame.height() - frame_top) / 2
        };

        copy(lock, {lumaPlaneParameters, chromaPlaneParameters});
    }

    void copy(VideoLock &lock, const CUDA_MEMCPY2D &parameters) {
        CUresult result;
        std::scoped_lock{lock};

        if ((result = cuMemcpy2D(&parameters)) != CUDA_SUCCESS) {
            throw std::runtime_error(std::to_string(result) + "EncodeBuffer.copy"); //TODO
        }
    }

    void copy(VideoLock &lock, const std::vector<CUDA_MEMCPY2D> &parameters) {
        std::scoped_lock{lock};
        std::for_each(parameters.begin(), parameters.end(), [](const CUDA_MEMCPY2D &parameters) {
            CUresult result;
            if ((result = cuMemcpy2D(&parameters)) != CUDA_SUCCESS) {
                throw std::runtime_error(std::to_string(result) + "EncodeBuffer.copy"); //TODO
            }
        });
    }

    void lock() {
        NVENCSTATUS status;
        if((status = encoder.api().NvEncMapInputResource(input_buffer.registered_resource,
                                                         &input_buffer.input_surface)) != NV_ENC_SUCCESS)
            throw std::runtime_error(std::to_string(status) + "EncodeBuffer.lock"); //TODO
    }

    void unlock() {
        NVENCSTATUS status;
        if((status = encoder.api().NvEncUnmapInputResource(input_buffer.input_surface)) != NV_ENC_SUCCESS)
            throw std::runtime_error(std::to_string(status) + "EncodeBuffer.unlock"); //TODO
    }
};

struct MotionEstimationBuffer
{
    EncodeOutputBuffer      output_buffer;
    EncodeInputBuffer       input_buffer[2];
    unsigned int            inputFrameIndex;
    unsigned int            referenceFrameIndex;
};

#endif //VISUALCLOUD_ENCODEBUFFER_H
