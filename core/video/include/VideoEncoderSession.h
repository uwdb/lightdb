#ifndef LIGHTDB_VIDEOENCODERSESSION_H
#define LIGHTDB_VIDEOENCODERSESSION_H

#include <errors.h>
#include "Frame.h"
#include "VideoEncoder.h"
#include "EncodeWriter.h"

struct VideoEncodePosition {
    const size_t x, y;
};

using FrameCopierFunction = std::function<void(EncodeBuffer&, Frame&, size_t)>;

class VideoEncoderSession {
public:
    VideoEncoderSession(VideoEncoder &encoder, EncodeWriter &writer)
        : frameCount_(0), encoder_(encoder), writer(writer), queue_(encoder_.buffers)
    { }

    VideoEncoderSession(const VideoEncoderSession &) = delete;
    VideoEncoderSession(VideoEncoderSession &&) = default;
    
    ~VideoEncoderSession() {
        Flush();
    }

    void Encode(const Frame &frame, size_t top=0, size_t left=0) {
        auto &buffer = GetAvailableBuffer();

        if(buffer.input_buffer.buffer_format != NV_ENC_BUFFER_FORMAT_NV12_PL)
            LOG(ERROR) << "buffer.input_buffer.buffer_format != NV_ENC_BUFFER_FORMAT_NV12_PL";
        assert(buffer.input_buffer.buffer_format == NV_ENC_BUFFER_FORMAT_NV12_PL);

        if(top == 0 && left == 0 &&
           frame.width() == buffer.input_buffer.width &&
           frame.height() == buffer.input_buffer.height)
            buffer.copy(encoder().lock_, frame);
        else
            buffer.copy(encoder().lock_, frame, top, left);
        return Encode(buffer, frame.type());
    }

    void Encode(std::vector<Frame> &frames, const FrameCopierFunction &copier) {
        return Encode(frames, copier, [](VideoLock&, EncodeBuffer& buffer) -> EncodeBuffer& { return buffer; });
    }

    void Encode(std::vector<Frame> &frames, const FrameCopierFunction &copier, const EncodableFrameTransform transform) {
        auto &buffer = GetAvailableBuffer();

        if(frames.empty())
            LOG(ERROR) << "no frames";
        else if(buffer.input_buffer.buffer_format != NV_ENC_BUFFER_FORMAT_NV12_PL)
            LOG(ERROR) << "buffer.input_buffer.buffer_format != NV_ENC_BUFFER_FORMAT_NV12_PL";
        assert(buffer.input_buffer.buffer_format == NV_ENC_BUFFER_FORMAT_NV12_PL);
        assert(!frames.empty());

        for(auto i = 0u; i < frames.size(); i++)
            copier(buffer, frames[i], i);
        return Encode(transform(encoder().lock_, buffer), frames[0].type());
    }

    void Flush() {
        NVENCSTATUS status;

        while(CompletePendingBuffer().has_value())
            std::this_thread::yield();

        if((status = encoder_.api().NvEncFlushEncoderQueue(nullptr)) != NV_ENC_SUCCESS)
            throw GpuEncodeRuntimeError("Encoder session failed to flush", status);

        writer.Flush();
    }

    const VideoEncoder &encoder() const { return encoder_; }
    size_t frameCount() const { return frameCount_; }

protected:
    size_t frameCount_;
    VideoEncoder &encoder_;
    EncodeWriter &writer;

private:
    template <class T> class EncoderBufferQueue {
        std::vector<T>& items;
        size_t pendingCount;
        size_t availableIndex;
        size_t pendingIndex;

    public:
        explicit EncoderBufferQueue(std::vector<T> &items)
            : items(items), pendingCount(0), availableIndex(0), pendingIndex(0)
        { }

        std::optional<T> GetAvailable() {
            if (pendingCount == items.size())
                return {};

            T& item = items[availableIndex];
            availableIndex = (availableIndex + 1) % items.size();
            pendingCount += 1;
            return item;
        }

        std::optional<T> GetPending() {
            if (pendingCount == 0)
                return {};

            T& item = items[pendingIndex];
            pendingIndex = (pendingIndex + 1) % items.size();
            pendingCount -= 1;
            return item;
        }
    };

    EncodeBuffer &GetAvailableBuffer() {
        std::optional<std::shared_ptr<EncodeBuffer>> buffer = queue_.GetAvailable();
        if(!buffer.has_value()) {
            CompletePendingBuffer();
            return *queue_.GetAvailable().value();
        } else
            return *buffer.value();
    }

    std::optional<std::shared_ptr<EncodeBuffer>> CompletePendingBuffer() {
        auto buffer = queue_.GetPending();

        if(buffer.has_value()) {
            writer.WriteFrame(*buffer.value());
            buffer->get()->input_buffer.input_surface = nullptr;
        }

        return {buffer};
    }

    void Encode(EncodeBuffer &buffer, NV_ENC_PIC_STRUCT type) {
        NVENCSTATUS status;

        std::scoped_lock lock{buffer};
        if ((status = encoder_.api().NvEncEncodeFrame(&buffer, nullptr, type, frameCount() == 0)) != NV_ENC_SUCCESS)
            throw GpuEncodeRuntimeError("Encoder session failed to encode input buffer", status);

        frameCount_++;
    }

    EncoderBufferQueue<std::shared_ptr<EncodeBuffer>> queue_;
};


#endif //LIGHTDB_VIDEOENCODESESSION_H
