#ifndef VISUALCLOUD_JOINVIDEOENCODER_H
#define VISUALCLOUD_JOINVIDEOENCODER_H

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "VideoEncoderSession.h"
#include "VideoDecoderSession.h"
#include "DecodeReader.h"
#include "EncodeWriter.h"
#include "ThreadPool.h"
#include "FrameRateAlignment.h"
#include <vector>
#include <numeric>

class JoinVideoEncoder {
public:
    JoinVideoEncoder(GPUContext& context,
                     const std::vector<DecodeConfiguration> &decodeConfigurations,
                     const EncodeConfiguration &encodeConfiguration,
                     const size_t rows, const size_t columns)
            : lock_(context),
              frameQueues_(CreateFrameQueues(lock_, rows * columns)),
              encodeConfiguration_(encodeConfiguration),
              decodeConfigurations_(decodeConfigurations.begin(), decodeConfigurations.end()),
              decoders_(CreateDecoders(decodeConfigurations_, frameQueues_, lock_, rows * columns)),
              encoder_(context, encodeConfiguration, lock_),
              rows_(rows), columns_(columns)
    {
        if(rows == 0 || columns == 0)
            throw std::runtime_error("bad rows/col"); //TODO
        else if(rows * columns != decodeConfigurations_.size())
            throw std::runtime_error("bad #decode configurations"); //TODO
    }
/*
    NVENCSTATUS tile(DecodeReader &reader, const std::vector<std::shared_ptr<EncodeWriter>> &writers) {
        return tile(reader, writers, [](VideoLock&, Frame& frame) -> Frame& { return frame; });
    }

    NVENCSTATUS tile(DecodeReader &reader, const std::vector<std::shared_ptr<EncodeWriter>> &writers,
                     std::vector<FrameTransform> transforms) {
        return tile(reader, writers, [this, transforms](VideoLock&, Frame& frame) -> Frame& {
            return std::accumulate(
                    transforms.begin(), transforms.end(),
                    std::ref(frame),
                    [this](auto& frame, auto& f) -> Frame& { return f(lock_, frame); });
        });
    }
*/
    NVENCSTATUS join(const std::vector<std::shared_ptr<DecodeReader>> &readers, EncodeWriter &writer) {
        auto decodeSessions = CreateDecodeSessions(readers);
        auto encoderSession = VideoEncoderSession(encoder_, writer);
        size_t framesDecoded = 0, framesEncoded = 0;

        //TODO assumes all decoders have the same framerate
        FrameRateAlignment alignment(encoder_.configuration().framerate, decoders_[0]->configuration().framerate);

        if(readers.size() != decoders_.size())
            throw std::runtime_error("bad size"); //TODO

        LOG(INFO) << "Joining starting (" << rows() << 'x' << columns() << ')';

        while (!decoders_[0]->frame_queue().isComplete()) {
            std::vector<Frame> decodedFrames, processedFrames;

            auto dropOrDuplicate = alignment.dropOrDuplicate(framesDecoded++, framesEncoded);

            for(auto &session: decodeSessions)
                decodedFrames.emplace_back(session->decode());

            try {
            for (auto i = 0u; i <= dropOrDuplicate; i++, framesEncoded++) {
                encoderSession.Encode(decodedFrames, [this](EncodeBuffer &buffer, Frame &frame, size_t index) {
                    auto row = index / columns(), column = index % columns();
                    buffer.copy(lock_, frame, 0, 0, row * frame.height(), column * frame.width());
                });
            }
            } catch(const std::exception& e) {
                LOG(INFO) << "foo" << e.what();
            }
        }

        LOG(INFO) << "Join complete (decoded " << framesDecoded << ", encoded " << framesEncoded << ")";

        return NV_ENC_SUCCESS;
    }

    EncodeAPI& api() { return encoder_.api(); } //TODO
    size_t rows() const { return rows_; }
    size_t columns() const { return columns_; }

private:
    VideoLock lock_;
    const std::vector<std::shared_ptr<CUVIDFrameQueue>> frameQueues_;
    const EncodeConfiguration encodeConfiguration_;
    const std::vector<DecodeConfiguration> decodeConfigurations_;
    std::vector<std::shared_ptr<CudaDecoder>> decoders_;
    VideoEncoder encoder_;
    const size_t rows_, columns_;

    static const std::vector<std::shared_ptr<CUVIDFrameQueue>> CreateFrameQueues(VideoLock &lock, size_t count) {
        std::vector<std::shared_ptr<CUVIDFrameQueue>> queues;

        queues.reserve(count);
        std::generate_n(std::back_inserter(queues), count, [&lock]() { return std::make_shared<CUVIDFrameQueue>(lock.get()); });

        return queues;
    }

    static const std::vector<std::shared_ptr<CudaDecoder>> CreateDecoders(
            const std::vector<DecodeConfiguration>& configurations,
            const std::vector<std::shared_ptr<CUVIDFrameQueue>> frameQueues,
            VideoLock &lock, size_t count) {
        std::vector<std::shared_ptr<CudaDecoder>> decoders;

        decoders.reserve(count);
        for(auto i = 0u; i < count; i++)
            decoders.emplace_back(std::make_shared<CudaDecoder>(configurations[i], *frameQueues[i], lock));
        //std::generate_n(std::back_inserter(decoders), count,
        //                [&configurations, &frameQueues, &lock, i]() mutable {
        //                    return std::make_shared<CudaDecoder>(configurations[i], *frameQueues[i++], lock); });

        return decoders;
    }

    const std::vector<std::shared_ptr<VideoDecoderSession>> CreateDecodeSessions(
            const std::vector<std::shared_ptr<DecodeReader>> &readers) {
        std::vector<std::shared_ptr<VideoDecoderSession>> sessions;

        sessions.reserve(readers.size());
        for(auto i = 0u; i < readers.size(); i++)
            sessions.emplace_back(std::make_shared<VideoDecoderSession>(*decoders_[i], *readers[i]));

        return std::move(sessions);
    }
};

#endif //VISUALCLOUD_JOINVIDEOENCODER_H
