#ifndef LIGHTDB_FRAME_H
#define LIGHTDB_FRAME_H

#include "VideoDecoder.h"
#include "lazy.h"
#include "utility"
#include "errors.h"
#include "dynlink_nvcuvid.h"

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

class DecodedFrame : public Frame {
public:
    DecodedFrame(const CudaDecoder& decoder, const std::shared_ptr<CUVIDPARSERDISPINFO> &parameters)
        : Frame(decoder.configuration(), extract_type(parameters)), decoder_(decoder), parameters_(parameters)
    { }

    DecodedFrame(const DecodedFrame&) = default;
    DecodedFrame(DecodedFrame &&other) noexcept = default;

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


class CudaFrame: public Frame {
public:
    CudaFrame(const Frame &frame, const CUdeviceptr handle, const unsigned int pitch)
            : Frame(frame), handle_(handle), pitch_(pitch)
    { }

    CudaFrame(const unsigned int height, const unsigned int width,
              const NV_ENC_PIC_STRUCT type, const CUdeviceptr handle, const unsigned int pitch)
            : Frame(height, width, type), handle_(handle), pitch_(pitch)
    { }

    virtual CUdeviceptr handle() const { return handle_; }
    virtual unsigned int pitch() const { return pitch_; }

protected:
    CudaFrame(const Frame &frame, const std::pair<CUdeviceptr, unsigned int> pair)
            : CudaFrame(frame, pair.first, pair.second)
    { }

private:
    const CUdeviceptr handle_;
    const unsigned int pitch_;
};

class CudaDecodedFrame: public DecodedFrame, public CudaFrame {
public:
    //TODO remove this overload after cleaning up hierarchy
    explicit CudaDecodedFrame(const Frame &frame)
            : CudaDecodedFrame(dynamic_cast<const DecodedFrame&>(frame))
    { }

    explicit CudaDecodedFrame(const DecodedFrame &frame)
            : DecodedFrame(frame), CudaFrame(frame, map_frame(frame))
    { }

    CudaDecodedFrame(CudaDecodedFrame &) noexcept = delete;
    CudaDecodedFrame(CudaDecodedFrame &&) noexcept = default;

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
