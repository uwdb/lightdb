#ifndef VISUALCLOUD_VIDEOENCODESESSION_H
#define VISUALCLOUD_VIDEOENCODESESSION_H

#include "VideoEncoder.h"
#include "EncodeWriter.h"

typedef struct _EncoderSessionInputFrame {
    CUdeviceptr  handle;
    unsigned int pitch;
    unsigned int width;
    unsigned int height;
} EncoderSessionInputFrame;


class VideoEncoderSession {
public:
    VideoEncoderSession(VideoEncoder &encoder, EncodeWriter &writer)
            : encoder(encoder), writer(writer), frameCount_(0), queue(encoder.buffers) { }

    ~VideoEncoderSession() {
        Flush();
    }

    NVENCSTATUS Encode(EncoderSessionInputFrame&, NV_ENC_PIC_STRUCT type = NV_ENC_PIC_STRUCT_FRAME);
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

            T&  item = items[availableIndex];
            availableIndex = (availableIndex + 1) % items.size();
            pendingCount += 1;
            return item;
        }

        std::optional<std::reference_wrapper<T>> GetPending() {
            if (pendingCount == 0)
                return {};

            T &item = items[pendingIndex];
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
