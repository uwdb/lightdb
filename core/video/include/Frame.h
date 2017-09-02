#ifndef VISUALCLOUD_FRAME_H
#define VISUALCLOUD_FRAME_H

#include "dynlink_nvcuvid.h"
#include "VideoDecoder.h"
#include "VideoEncoder.h"

class Frame {
public:
    Frame(Frame &frame)
        : Frame(frame.handle(), frame.pitch(), frame.height(), frame.width(), frame.type())
    { }

    Frame(unsigned int height, unsigned int width, NV_ENC_PIC_STRUCT type)
        : Frame(0, 0, height, width, type)
    { }

    Frame(const EncodeConfig &configuration, NV_ENC_PIC_STRUCT type)
        : Frame(0, 0, configuration, type)
    { }

    Frame(CUdeviceptr handle, unsigned int pitch, const EncodeConfig &configuration, NV_ENC_PIC_STRUCT type)
        : Frame(handle, pitch, configuration.height, configuration.width, type)
    { }

    Frame(CUdeviceptr handle, unsigned int pitch, unsigned int height, unsigned int width,
          NV_ENC_PIC_STRUCT type)
            : handle_(handle), pitch_(pitch), height_(height), width_(width), type_(type)
    { }

    CUdeviceptr handle() const { return handle_; }
    unsigned int pitch() const { return pitch_; }
    unsigned int height() const { return height_; }
    unsigned int width() const { return width_; }
    NV_ENC_PIC_STRUCT type() const { return type_; }
/*
    Frame&& crop(VideoLock &lock, size_t top, size_t left, size_t height, size_t width) {
        CUDA_MEMCPY2D lumaPlaneParameters = {
                srcXInBytes:   left,
                srcY:          top,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       NULL,
                srcDevice:     handle(),
                srcArray:      NULL,
                srcPitch:      pitch(),

                dstXInBytes:   0,
                dstY:          0,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       NULL,
                dstDevice:     (CUdeviceptr)encodeBuffer->stInputBfr.pNV12devPtr,
                dstArray:      NULL,
                dstPitch:      encodeBuffer->stInputBfr.uNV12Stride,

                WidthInBytes:  width,
                Height:        height,
        };

        CUDA_MEMCPY2D chromaPlaneParameters = {
                srcXInBytes:   left,
                srcY:          this->height() + top/2,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       NULL,
                srcDevice:     handle(),
                srcArray:      NULL,
                srcPitch:      pitch(),

                dstXInBytes:   0,
                dstY:          height,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       NULL,
                dstDevice:     (CUdeviceptr)encodeBuffer->stInputBfr.pNV12devPtr,
                dstArray:      NULL,
                dstPitch:      encodeBuffer->stInputBfr.uNV12Stride,

                WidthInBytes:  width,
                Height:        height / 2
        };

        std::lock_guard(lock);

        if((result = cuMemcpy2D(&lumaPlaneParameters)) != CUDA_SUCCESS)
            return error("cuMemcpy2D", result, NV_ENC_ERR_GENERIC);
        else if((result = cuMemcpy2D(&chromaPlaneParameters)) != CUDA_SUCCESS)
            return error("cuMemcpy2D", result, NV_ENC_ERR_GENERIC);
        else if((status = context.hardwareEncoder.NvEncMapInputResource(
                encodeBuffer->stInputBfr.nvRegisteredResource,
                &encodeBuffer->stInputBfr.hInputSurface)) != NV_ENC_SUCCESS)
            return status;
        else
            context.hardwareEncoder.NvEncEncodeFrame(encodeBuffer, NULL, inputFrameType);
    }
*/

protected:
    CUdeviceptr handle_;
    unsigned int pitch_;
    unsigned int height_, width_;
    NV_ENC_PIC_STRUCT type_;
};

class DecodedFrame : public Frame {
public:
    DecodedFrame(const CudaDecoder& decoder, const std::shared_ptr<CUVIDPARSERDISPINFO> parameters)
        : Frame(decoder.configuration(), extract_type(parameters)),
          decoder_(decoder), parameters_(parameters)
    {
        CUresult result;
        CUVIDPROCPARAMS mapParameters{
                .progressive_frame = parameters->progressive_frame,
                .second_field = 0,
                .top_field_first = parameters->top_field_first,
                .unpaired_field = parameters->progressive_frame == 1 || parameters->repeat_first_field <= 1};

        if((result = cuvidMapVideoFrame(decoder_.handle(), parameters->picture_index,
                                        &handle_, &pitch_, &mapParameters)) != CUDA_SUCCESS)
            throw result; //TODO
    }

    ~DecodedFrame() {
        if(handle())
            cuvidUnmapVideoFrame(decoder_.handle(), handle());
    }

    const std::shared_ptr<CUVIDPARSERDISPINFO> parameters() const { return parameters_; }
    virtual unsigned int height() const { return decoder_.configuration().height; }
    virtual unsigned int width() const { return decoder_.configuration().width; }

private:
    static NV_ENC_PIC_STRUCT extract_type(const std::shared_ptr<CUVIDPARSERDISPINFO> &parameters) {
        return (parameters == nullptr || parameters->progressive_frame || parameters->repeat_first_field >= 2
                ? NV_ENC_PIC_STRUCT_FRAME
                : (parameters->top_field_first
                      ? NV_ENC_PIC_STRUCT_FIELD_TOP_BOTTOM
                      : NV_ENC_PIC_STRUCT_FIELD_BOTTOM_TOP));
    }

    const CudaDecoder &decoder_;
    const std::shared_ptr<CUVIDPARSERDISPINFO> parameters_;
};
/*
class CroppedFrame : public Frame {
public:
    CroppedFrame(GPUContext &context, VideoLock &lock, const Frame& sourceFrame,
                 size_t top, size_t left, size_t height, size_t width)
        : context(context)
    {

    }

    ~CroppedFrame() {
        if((status = context.hardwareEncoder.NvEncUnmapInputResource(
                encodeBuffer->stInputBfr.nvRegisteredResource,
                &encodeBuffer->stInputBfr.hInputSurface)) != NV_ENC_SUCCESS)
            return status;

    }

private:
    GPUContext &context;
};
*/

#endif //VISUALCLOUD_FRAME_H
