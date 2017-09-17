#ifndef VISUALCLOUD_VIDEOENCODERSESSION_H
#define VISUALCLOUD_VIDEOENCODERSESSION_H

#include "Frame.h"
#include "VideoEncoder.h"
#include "EncodeWriter.h"

class VideoEncoderSession {
public:
    VideoEncoderSession(VideoEncoder &encoder, EncodeWriter &writer)
        : frameCount_(0), encoder_(encoder), writer(writer), queue_(encoder_.buffers)
    { }

    ~VideoEncoderSession() {
        Flush();
    }

    void Encode(Frame &frame) {
        auto &buffer = GetAvailableBuffer();

        buffer.copy(encoder().lock, frame);
        return Encode(frame, buffer);
    }

    void Encode(Frame &frame, size_t top, size_t left) {
        auto &buffer = GetAvailableBuffer();

        assert(buffer.stInputBfr.bufferFmt == NV_ENC_BUFFER_FORMAT_NV12_PL);

        buffer.copy(encoder().lock, frame, top, left);
        return Encode(frame, buffer);
    }

    void Flush() {
        NVENCSTATUS status;

        while(CompletePendingBuffer().has_value())
            std::this_thread::yield();

        if((status = encoder_.api().NvEncFlushEncoderQueue(nullptr)) != NV_ENC_SUCCESS)
            throw std::runtime_error(std::to_string(status)); //TODO

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
        EncoderBufferQueue(std::vector<T> &items)
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
            buffer->get()->stInputBfr.hInputSurface = nullptr;
        }

        return buffer;
    }

    void Encode(Frame &frame, EncodeBuffer &buffer) {
        NVENCSTATUS status;

        std::scoped_lock{buffer};
        if ((status = encoder_.api().NvEncEncodeFrame(&buffer, nullptr, frame.type(), frameCount() == 0)) != NV_ENC_SUCCESS)
            throw std::runtime_error(std::to_string(status)); //TODO

        frameCount_++;
    }

    EncoderBufferQueue<std::shared_ptr<EncodeBuffer>> queue_;
};


#endif //VISUALCLOUD_VIDEOENCODESESSION_H
