#ifndef LIGHTDB_FRAME_H
#define LIGHTDB_FRAME_H

#include "VideoDecoder.h"
#include "Encoding.h"
#include "lazy.h"
#include "reference.h"
#include "utility"
#include "errors.h"
#include <mutex>
#include <utility>

//TODO move into lightdb namespace

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
class LocalFrame;

class GPUFrame : public Frame {
public:
    using Frame::Frame;
    explicit GPUFrame(const Frame& frame) : Frame(frame) { }
    explicit GPUFrame(Frame && frame) noexcept : Frame(frame) { }

public:
    virtual std::shared_ptr<CudaFrame> cuda() = 0;
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

    explicit CudaFrame(const LocalFrame& frame);

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

    std::shared_ptr<CudaFrame> cuda() override {
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
using CudaFrameReference = lightdb::shared_reference<CudaFrame>;

class DecodedFrame : public GPUFrame {
public:
    DecodedFrame(const CudaDecoder& decoder, const std::shared_ptr<CUVIDPARSERDISPINFO> &parameters)
        : GPUFrame(decoder.configuration(), extract_type(parameters)), decoder_(decoder), parameters_(parameters)
    { }

    DecodedFrame(const DecodedFrame&) = default;
    DecodedFrame(DecodedFrame &&other) noexcept = default;

    std::shared_ptr<CudaFrame> cuda() override;

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
    std::shared_ptr<CudaFrame> cuda_;
};

class CudaDecodedFrame: public DecodedFrame, public CudaFrame {
public:
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
    static std::pair<CUdeviceptr, unsigned int> map_frame(const DecodedFrame &frame) {
        CUresult result;
        CUdeviceptr handle;
        unsigned int pitch;
        CUVIDPROCPARAMS mapParameters{
                .progressive_frame = frame.parameters().progressive_frame,
                .second_field = 0,
                .top_field_first = frame.parameters().top_field_first,
                .unpaired_field = frame.parameters().progressive_frame == 1 || frame.parameters().repeat_first_field <= 1,
                0, 0, 0, 0, 0, 0, 0, {}, {}};
        if((result = cuvidMapVideoFrame(frame.decoder().handle(), frame.parameters().picture_index,
                                        &handle, &pitch, &mapParameters)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuvidMapVideoFrame failed", result);

        return std::make_pair(handle, pitch);
    }
};

class LocalFrame: public Frame {
public:
    LocalFrame(const LocalFrame &frame)
            : LocalFrame(frame, frame.data_)
    { }

    LocalFrame(const LocalFrame &frame, const lightdb::bytestring &data)
            : LocalFrame(frame, std::make_shared<lightdb::bytestring>(data.begin(), data.end()))
    { }

    LocalFrame(unsigned int height, unsigned int width, const lightdb::bytestring &data)
            : LocalFrame(height, width, std::make_shared<lightdb::bytestring>(data.begin(), data.end()))
    { }

    LocalFrame(unsigned int height, unsigned int width, std::shared_ptr<lightdb::bytestring> data)
            : Frame(height, width, NV_ENC_PIC_STRUCT_FRAME),
              data_(std::move(data))
    { }

    LocalFrame(const LocalFrame &frame, const size_t size)
            : LocalFrame(frame, std::make_shared<lightdb::bytestring>(size, 0))
    { }

    LocalFrame(const LocalFrame &frame, std::shared_ptr<lightdb::bytestring> data)
            : Frame(frame),
              data_(std::move(data))
    { }

    explicit LocalFrame(const CudaFrame &source)
    : LocalFrame(source, Configuration{source.width(), source.height(), 0, 0, 0, {0, 1}, {0, 0}}) { }

    explicit LocalFrame(const CudaFrame &source, const Configuration &configuration)
        : Frame(configuration, NV_ENC_PIC_STRUCT_FRAME),
          data_(std::make_shared<lightdb::bytestring>(width() * height() * 3 / 2, 0))
    {
        CUresult status;
        auto params = CUDA_MEMCPY2D {
            .srcXInBytes = configuration.offset.left,
            .srcY = configuration.offset.top,
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

    virtual unsigned char operator()(size_t x, size_t y) const { return data_->at(x + y * width()); }
    const lightdb::bytestring& data() const { return *data_; }

private:
    const std::shared_ptr<lightdb::bytestring> data_;
};

//TODO this should be CPUFrameRef
using LocalFrameReference = lightdb::shared_reference<LocalFrame>;

#endif //LIGHTDB_FRAME_H
