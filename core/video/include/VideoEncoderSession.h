#ifndef VISUALCLOUD_VIDEOENCODERSESSION_H
#define VISUALCLOUD_VIDEOENCODERSESSION_H

#include "Frame.h"
#include "VideoEncoder.h"
#include "EncodeWriter.h"

class VideoEncoderSession {
public:
    VideoEncoderSession(VideoEncoder &encoder, EncodeWriter &writer)
            : frameCount_(0), encoder(encoder), writer(writer), queue(encoder.buffers) { }

    ~VideoEncoderSession() {
        Flush();
    }

    //TODO case
    NVENCSTATUS Encode(Frame&);
    NVENCSTATUS Flush();

    size_t frameCount() const { return frameCount_; }

protected:
    size_t frameCount_;

    VideoEncoder &encoder;
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

        std::optional<std::reference_wrapper<T>> GetAvailable() {
            if (pendingCount == items.size())
                return {};

            T& item = items[availableIndex];
            availableIndex = (availableIndex + 1) % items.size();
            pendingCount += 1;
            return item;
        }

        std::optional<std::reference_wrapper<T>> GetPending() {
            if (pendingCount == 0)
                return {};

            T& item = items[pendingIndex];
            pendingIndex = (pendingIndex + 1) % items.size();
            pendingCount -= 1;
            return item;
        }
    };

    EncodeBuffer &GetAvailableBuffer();
    std::optional<std::reference_wrapper<EncodeBuffer>> CompletePendingBuffer();
    EncoderBufferQueue<EncodeBuffer> queue;
};


#endif //VISUALCLOUD_VIDEOENCODESESSION_H
