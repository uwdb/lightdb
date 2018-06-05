#ifndef LIGHTDB_FRAME_H
#define LIGHTDB_FRAME_H

#include "VideoDecoder.h"
#include "lazy.h"
#include "reference.h"
#include "utility"
#include "errors.h"
#include "dynlink_nvcuvid.h"
#include <mutex>

class Frame {
public:
    Frame(unsigned int height, unsigned int width, NV_ENC_PIC_STRUCT type)
        : height_(height), width_(width), type_(type)
    { }

    Frame(const Configuration &configuration, NV_ENC_PIC_STRUCT type)
        : Frame(configuration.height, configuration.width, type)
    { }

    Frame(const Frame&) = default;
    Frame(Frame &&) noexcept = default;

    virtual ~Frame() = default;

    virtual unsigned int height() const { return height_; }
    virtual unsigned int width() const { return width_; }
    //TODO move this to GPUFrame
    NV_ENC_PIC_STRUCT type() const { return type_; }

protected:
    const unsigned int height_, width_;
    NV_ENC_PIC_STRUCT type_;
};

class CudaFrame;
class GPUFrame : public Frame {
public:
    using Frame::Frame;
    explicit GPUFrame(const Frame& frame) : Frame(frame) { }
    explicit GPUFrame(Frame && frame) noexcept : Frame(frame) { }

public:
    virtual std::shared_ptr<CudaFrame> cuda() const = 0;
};
using GPUFrameReference = lightdb::shared_reference<GPUFrame>;

class CudaFrame: public GPUFrame {
public:
    explicit CudaFrame(const Frame &frame)
            : CudaFrame(frame, std::tuple_cat(allocate_frame(frame), std::make_tuple(true)))
    { }

    CudaFrame(const Frame &frame, const CUdeviceptr handle, const unsigned int pitch)
            : CudaFrame(frame, handle, pitch, false)
    { }

    CudaFrame(const unsigned int height, const unsigned int width, const NV_ENC_PIC_STRUCT type)
            : CudaFrame(Frame{height, width, type})
    { }

    CudaFrame(const unsigned int height, const unsigned int width,
              const NV_ENC_PIC_STRUCT type, const CUdeviceptr handle, const unsigned int pitch)
            : GPUFrame(height, width, type), handle_(handle), pitch_(pitch), owner_(false)
    { }

    CudaFrame(const CudaFrame&) = delete;
    CudaFrame(CudaFrame&& frame) noexcept
            : GPUFrame(std::move(frame)), handle_(frame.handle_), pitch_(frame.pitch_), owner_(frame.owner_)
    { frame.owner_ = false; }

    ~CudaFrame() override {
        CUresult result;

        if(owner_ && (result = cuMemFree(handle_)) != CUDA_SUCCESS)
            LOG(ERROR) << "Swallowed failure to free CudaFrame resources (" << result << ")";
    }

    virtual CUdeviceptr handle() const { return handle_; }
    virtual unsigned int pitch() const { return pitch_; }

    std::shared_ptr<CudaFrame> cuda() const override {
        //owner_ = false;
        //handle_ = 0;
        //pitch_ = 0;
        //TODO this is buggy?
        return std::make_shared<CudaFrame>(*this, handle_, pitch_);
    }

    void copy(VideoLock &lock, const CudaFrame &frame) {
        if(frame.width() != width() ||
           frame.height() != height()) {
            throw InvalidArgumentError("Frame sizes do not match", "frame");
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
            .dstDevice = handle(),
            .dstArray = nullptr,
            .dstPitch = pitch(),

            .WidthInBytes = width(),
            .Height = height() * 3 / 2 //TODO this assumes NV12 format
        });
    }

    void copy(VideoLock &lock, const CudaFrame &source,
              size_t source_top, size_t source_left,
              size_t destination_top=0, size_t destination_left=0) {

        CUDA_MEMCPY2D lumaPlaneParameters = {
                srcXInBytes:   source_left,
                srcY:          source_top,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       nullptr,
                srcDevice:     source.handle(),
                srcArray:      nullptr,
                srcPitch:      source.pitch(),

                dstXInBytes:   destination_left,
                dstY:          destination_top,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       nullptr,
                dstDevice:     handle(),
                dstArray:      nullptr,
                dstPitch:      pitch(),

                WidthInBytes:  std::min(width() - destination_left, source.width() - source_left),
                Height:        std::min(height() - destination_top, source.height() - source_top) ,
        };

        CUDA_MEMCPY2D chromaPlaneParameters = {
                srcXInBytes:   source_left,
                srcY:          source.height() + source_top / 2,
                srcMemoryType: CU_MEMORYTYPE_DEVICE,
                srcHost:       nullptr,
                srcDevice:     source.handle(),
                srcArray:      nullptr,
                srcPitch:      source.pitch(),

                dstXInBytes:   destination_left,
                dstY:          height() + destination_top / 2,
                dstMemoryType: CU_MEMORYTYPE_DEVICE,
                dstHost:       nullptr,
                dstDevice:     handle(),
                dstArray:      nullptr,
                dstPitch:      pitch(),

                WidthInBytes:  std::min(width() - destination_left, source.width() - source_left),
                Height:        std::min(height() - destination_top, source.height() - source_top) / 2
        };

        copy(lock, {lumaPlaneParameters, chromaPlaneParameters});
    }

protected:
    CudaFrame(const Frame &frame, const std::pair<CUdeviceptr, unsigned int> pair)
            : CudaFrame(frame, pair.first, pair.second, false)
    { }

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

private:
    CudaFrame(const Frame &frame, const std::tuple<CUdeviceptr, unsigned int, bool> tuple)
            : CudaFrame(frame, std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple))
    { }

    CudaFrame(const Frame &frame, const CUdeviceptr handle, const unsigned int pitch, const bool owner)
            : GPUFrame(frame), handle_(handle), pitch_(pitch), owner_(owner)
    { }

    static std::pair<CUdeviceptr, unsigned int> allocate_frame(const Frame &frame)
    {
        CUresult result;
        CUdeviceptr handle;
        size_t pitch;

        if((result = cuMemAllocPitch(&handle,
                                     &pitch,
                                     frame.width(),
                                     frame.height() * 3 / 2,
                                     16)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuMemAllocPitch failed", result);
        else
            return std::make_pair(handle, static_cast<unsigned int>(pitch));
    }

    const CUdeviceptr handle_;
    const unsigned int pitch_;
    bool owner_;
};

class DecodedFrame : public GPUFrame {
public:
    DecodedFrame(const CudaDecoder& decoder, const std::shared_ptr<CUVIDPARSERDISPINFO> &parameters)
        : GPUFrame(decoder.configuration(), extract_type(parameters)), decoder_(decoder), parameters_(parameters)
    { }

    DecodedFrame(const DecodedFrame&) = default;
    DecodedFrame(DecodedFrame &&other) noexcept = default;

    std::shared_ptr<CudaFrame> cuda() const override;

    const CudaDecoder &decoder() const { return decoder_; }
    const CUVIDPARSERDISPINFO& parameters() const { return *parameters_; }
    unsigned int height() const override { return decoder_.configuration().height; }
    unsigned int width() const override { return decoder_.configuration().width; }

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

class CudaDecodedFrame: public DecodedFrame, public CudaFrame {
public:
    //TODO remove this overload after cleaning up hierarchy
    //explicit CudaDecodedFrame(const Frame &frame)
    //        : CudaDecodedFrame(dynamic_cast<const DecodedFrame&>(frame))
    //{ }

    explicit CudaDecodedFrame(const DecodedFrame &frame)
            : DecodedFrame(frame), CudaFrame(frame, map_frame(frame))
    { }

    CudaDecodedFrame(CudaDecodedFrame &) noexcept = delete;
    CudaDecodedFrame(CudaDecodedFrame &&) noexcept = delete;

    ~CudaDecodedFrame() override {
        CUresult result = cuvidUnmapVideoFrame(decoder().handle(), handle());
        if(result != CUDA_SUCCESS)
            LOG(WARNING) << "Ignoring error << " << result << " in cuvidUnmapVideoFrame destructor.";
    }

    unsigned int height() const override { return DecodedFrame::height(); }
    unsigned int width() const override { return DecodedFrame::width(); }

private:
    static std::pair<CUdeviceptr, unsigned int> map_frame(const DecodedFrame &frame)
    {
        CUresult result;
        CUdeviceptr handle;
        unsigned int pitch;
        CUVIDPROCPARAMS mapParameters{
                .progressive_frame = frame.parameters().progressive_frame,
                .second_field = 0,
                .top_field_first = frame.parameters().top_field_first,
                .unpaired_field = frame.parameters().progressive_frame == 1 || frame.parameters().repeat_first_field <= 1};
        if((result = cuvidMapVideoFrame(frame.decoder().handle(), frame.parameters().picture_index,
                                        &handle, &pitch, &mapParameters)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuvidMapVideoFrame failed", result);

        return std::make_pair(handle, pitch);
    }
};


//TODO really should have a base class Frame for host and GPU frames, and then change the current Frame class to GpuFrame
class LocalFrame {
public:
    LocalFrame(const LocalFrame &frame)
        : height_(frame.height()), width_(frame.width()), data_(frame.data_)
    { }

    explicit LocalFrame(const CudaFrame &source)
        : height_(source.height()), width_(source.width()),
          data_(std::make_shared<std::vector<unsigned char>>(width() * height() * 3 / 2))
    {
        CUresult status;
        auto params = CUDA_MEMCPY2D {
            .srcXInBytes = 0,
            .srcY = 0,
            .srcMemoryType = CU_MEMORYTYPE_DEVICE,
            .srcHost = nullptr,
            .srcDevice = source.handle(),
            .srcArray = nullptr,
            .srcPitch = source.pitch(),

            .dstXInBytes = 0,
            .dstY = 0,

            .dstMemoryType = CU_MEMORYTYPE_HOST,
            .dstHost = data_->data(),
            .dstDevice = 0,
            .dstArray = nullptr,
            .dstPitch = 0,

            .WidthInBytes = width(),
            .Height = height() * 3 / 2
        };

        if((status = cuMemcpy2D(&params)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuMemcpy2D failed", status);
    }

    virtual unsigned int height() const { return height_; }
    virtual unsigned int width() const { return width_; }

    unsigned char operator()(size_t x, size_t y) const { return data_->at(x + y * width()); }

private:
    const unsigned int height_, width_;
    const std::shared_ptr<std::vector<unsigned char>> data_;
};

//TODO Make EncodeBuffer a "EncodableFrame" derived from Frame
class EncodeBuffer;
typedef std::function<EncodeBuffer&(VideoLock&, EncodeBuffer&)> EncodableFrameTransform;
typedef std::function<const Frame&(VideoLock&, const Frame&)> FrameTransform;
typedef std::function<const Frame&(VideoLock&, const std::vector<Frame>&)> NaryFrameTransform;

#endif //LIGHTDB_FRAME_H
