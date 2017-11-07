#ifndef _TILE_VIDEO_ENCODER
#define _TILE_VIDEO_ENCODER

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

class TileVideoEncoder {
public:
    TileVideoEncoder(GPUContext& context,
                     const DecodeConfiguration &decodeConfiguration,
                     const EncodeConfiguration &encodeModelConfiguration,
                     const size_t rows, const size_t columns)
            : TileVideoEncoder(context, decodeConfiguration,
                               {rows * columns, EncodeConfiguration(encodeModelConfiguration,
                                                                    decodeConfiguration.height / rows,
                                                                    decodeConfiguration.width / columns)},
                               rows, columns)
    { }

    TileVideoEncoder(GPUContext& context,
                     const DecodeConfiguration &decodeConfiguration,
                     const std::vector<EncodeConfiguration> &encodeConfigurations,
                     const size_t rows, const size_t columns)
            : lock_(context),
              frameQueue_(lock_.get()),
              encodeConfigurations_(encodeConfigurations.begin(), encodeConfigurations.end()),
              decoder_(decodeConfiguration, frameQueue_, lock_),
              encoders_(CreateEncoders(context, encodeConfigurations_, lock_, rows * columns)),
              rows_(rows), columns_(columns),
              pool(context, rows * columns)
    {
      if(rows == 0 || columns == 0)
        throw std::runtime_error("bad rows/col"); //TODO
      else if(rows * columns != encodeConfigurations_.size())
            throw std::runtime_error("bad #encode configurations"); //TODO
    }

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

#include <chrono>
    NVENCSTATUS tile(DecodeReader &reader, const std::vector<std::shared_ptr<EncodeWriter>> &writers,
                     FrameTransform transform) {
      VideoDecoderSession decodeSession(decoder_, reader);
      auto sessions = CreateEncoderSessions(writers);
      size_t framesDecoded = 0, framesEncoded = 0;
      FrameRateAlignment alignment(encoders()[0]->configuration().framerate, decoder_.configuration().framerate);

      if(writers.size() != encoders().size())
        throw std::runtime_error("bad size"); //TODO

      LOG(INFO) << "Tiling starting (" << rows() << 'x' << columns() << ')';

      while (!decoder_.frame_queue().isComplete()) {
        auto dropOrDuplicate = alignment.dropOrDuplicate(framesDecoded++, framesEncoded);
        auto decodedFrame = decodeSession.decode();
        auto processedFrame = transform(lock_, decodedFrame);

        for (auto i = 0u; i <= dropOrDuplicate; i++, framesEncoded++) {
          for(auto j = 0u; j < sessions.size(); j++) {
            auto &session = sessions[j];
            auto row = j / columns(), column = j % columns();

            pool.push([&session, &processedFrame, row, column]() {
                session.Encode(processedFrame,
                               row * session.encoder().configuration().height,
                               column * session.encoder().configuration().width);
            });
          }

          pool.waitAll();
        }
      }

      LOG(INFO) << "Tiling complete (decoded " << framesDecoded << ", encoded " << framesEncoded << ")";

      return NV_ENC_SUCCESS;
    }

    const std::vector<std::shared_ptr<VideoEncoder>> &encoders() const { return encoders_; }
    EncodeAPI& api() { return encoders()[0]->api(); } //TODO
    size_t rows() const { return rows_; }
    size_t columns() const { return columns_; }

private:
    VideoLock lock_;
    CUVIDFrameQueue frameQueue_;
    const std::vector<EncodeConfiguration> encodeConfigurations_;
    std::vector<std::shared_ptr<VideoEncoder>> encoders_;
    CudaDecoder decoder_; //TODO this should be a base VideoDecoder
    const size_t rows_, columns_;
    GPUThreadPool pool;

    static const std::vector<std::shared_ptr<VideoEncoder>> CreateEncoders(
            GPUContext& context, const std::vector<EncodeConfiguration>& configurations, VideoLock &lock, size_t count) {
      std::vector<std::shared_ptr<VideoEncoder>> encoders;
      size_t i = 0u;

      encoders.reserve(count);
      std::generate_n(std::back_inserter(encoders), count,
                      [&context, &configurations, &lock, i]() mutable {
                          return std::make_shared<VideoEncoder>(context, configurations.at(i++), lock); });

      return encoders;
    }

    const std::vector<VideoEncoderSession> CreateEncoderSessions(
            const std::vector<std::shared_ptr<EncodeWriter>> &writers) {
      std::vector<VideoEncoderSession> sessions;

      sessions.reserve(writers.size());
      std::transform(encoders_.begin(), encoders_.end(), writers.begin(), std::back_inserter(sessions),
                     [](std::shared_ptr<VideoEncoder> &encoder, std::shared_ptr<EncodeWriter> writer) {
                         return std::move(VideoEncoderSession(*encoder, *writer)); });

      return std::move(sessions);
    }
};

#endif
