#ifndef LIGHTDB_JOINVIDEOENCODER_H
#define LIGHTDB_JOINVIDEOENCODER_H

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
        if(rows == 0)
            throw InvalidArgumentError("Number of rows must be positive", "rows");
        else if(columns == 0)
            throw InvalidArgumentError("Number of columns must be positive", "columns");
        else if(rows * columns != decodeConfigurations_.size())
            throw InvalidArgumentError("Number of decode configurations must be rows*columns", "decodeConfigurations");
    }

    NVENCSTATUS join(const std::vector<std::shared_ptr<DecodeReader>> &readers, EncodeWriter &writer) {
        return join(readers, writer, [](VideoLock&, EncodeBuffer& buffer) -> EncodeBuffer& { return buffer; });
    }

    NVENCSTATUS join(const std::vector<std::shared_ptr<DecodeReader>> &readers, EncodeWriter &writer,
                     const std::vector<FrameTransform> tileTransforms) {
        return join(readers, writer, tileTransforms,
                    [](VideoLock&, EncodeBuffer& buffer) -> EncodeBuffer& { return buffer; });
    }

    NVENCSTATUS join(const std::vector<std::shared_ptr<DecodeReader>> &readers, EncodeWriter &writer,
                     EncodableFrameTransform joinedTransform) {
        return join(readers, writer,
                    std::vector<FrameTransform>{readers.size(), [](VideoLock&, Frame& frame) -> Frame& {
                        return frame; }},
                    joinedTransform);
    }

    NVENCSTATUS join(const std::vector<std::shared_ptr<DecodeReader>> &readers, EncodeWriter &writer,
                     const std::vector<FrameTransform> tileTransforms,
                     EncodableFrameTransform joinedTransform) {
        auto decodeSessions = CreateDecodeSessions(readers);
        auto encoderSession = VideoEncoderSession(encoder_, writer);
        size_t framesDecoded = 0, framesEncoded = 0;

        //TODO assumes all decoders have the same framerate
        FrameRateAlignment alignment(encoder_.configuration().framerate, decoders_[0]->configuration().framerate);

        if(readers.size() != decoders_.size())
            throw InvalidArgumentError("Number of readers must match number of decoders", "readers");
        else if(tileTransforms.size() != decoders_.size())
            throw InvalidArgumentError("Number of tile transforms must be equal to the number of decoders",
                                       "tileTransforms");

        LOG(INFO) << "Joining starting (" << rows() << " rows x " << columns() << " columns)";

        while (!decoders_[0]->frame_queue().isComplete()) {
            std::vector<Frame> decodedFrames, processedFrames;

            auto dropOrDuplicate = alignment.dropOrDuplicate(framesDecoded++, framesEncoded);

            for(auto &session: decodeSessions)
                decodedFrames.emplace_back(session->decode());

            for (auto i = 0; i <= dropOrDuplicate; i++, framesEncoded++) {
                encoderSession.Encode(decodedFrames,
                                      [tileTransforms, joinedTransform, this](
                                              EncodeBuffer &buffer, Frame &frame, size_t index) {
                    auto row = index / columns(), column = index % columns();
                    buffer.copy(lock_, tileTransforms[index](lock_, frame),
                                0, 0, row * frame.height(), column * frame.width());
                }, joinedTransform);
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
        auto i = 0u;

        std::generate_n(std::back_inserter(decoders), count,
                        [&configurations, &frameQueues, &lock, i]() mutable {
                            return std::make_shared<CudaDecoder>(configurations[i++], *frameQueues[i], lock); });

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

#endif //LIGHTDB_JOINVIDEOENCODER_H
