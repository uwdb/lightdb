#ifndef VISUALCLOUD_VIDEOENCODERSESSION_H
#define VISUALCLOUD_VIDEOENCODERSESSION_H

#include "Frame.h"
#include "VideoEncoder.h"
#include "EncodeWriter.h"

class VideoEncoderSession {
public:
    VideoEncoderSession(VideoEncoder &encoder, EncodeWriter &writer)
        : frameCount_(0), encoder_(encoder), writer(writer), queue(encoder_.buffers)
    { }

    ~VideoEncoderSession() {
        Flush();
    }

    //TODO case
    NVENCSTATUS Encode(Frame&);
    NVENCSTATUS Encode(Frame&, size_t top, size_t left);
    NVENCSTATUS Flush();

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

    EncodeBuffer &GetAvailableBuffer();
    std::optional<std::shared_ptr<EncodeBuffer>> CompletePendingBuffer();
    EncoderBufferQueue<std::shared_ptr<EncodeBuffer>> queue;
};


#endif //VISUALCLOUD_VIDEOENCODESESSION_H
