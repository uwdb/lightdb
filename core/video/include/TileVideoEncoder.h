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

class TileVideoEncoder {
public:
    TileVideoEncoder(GPUContext& context,
                     const DecodeConfiguration &decodeConfiguration,
                     const EncodeConfiguration &encodeModelConfiguration,
                     const size_t rows, const size_t columns)
            : lock_(context),
              encoderConfiguration_(encodeModelConfiguration,
                                    decodeConfiguration.height / rows,
                                    decodeConfiguration.width / columns),
              frameQueue_(lock_.get()),
              decoder_(decodeConfiguration, frameQueue_, lock_),
              encoders_(CreateEncoders(context, encoderConfiguration_, lock_, rows * columns)),
              rows_(rows), columns_(columns),
              pool(context, rows * columns)
    {
      if(rows == 0 || columns == 0)
        throw std::runtime_error("bad rows/col"); //TODO
    }

    NVENCSTATUS tile(DecodeReader &reader, const std::vector<std::shared_ptr<EncodeWriter>> &writers) {
      NVENCSTATUS status;
      VideoDecoderSession decodeSession(decoder_, reader);
      auto sessions = CreateEncoderSessions(writers);
      size_t framesDecoded = 0, framesEncoded = 0;
      FrameRateAlignment alignment(encoders()[0]->configuration().framerate, decoder_.configuration().framerate);

      if(writers.size() != encoders().size())
        throw std::runtime_error("bad size"); //TODO

      while (!decoder_.frame_queue().isComplete()) {
        auto dropOrDuplicate = alignment.dropOrDuplicate(framesDecoded++, framesEncoded);
        auto decodedFrame = decodeSession.decode();

        for (auto i = 0; i <= dropOrDuplicate; i++, framesEncoded++) {
          for(auto j = 0; j < sessions.size(); j++) {
            auto &session = sessions[j];
            auto row = j / columns(), column = j % columns();

            pool.push([&session, &decodedFrame, row, column]() {
                session.Encode(decodedFrame,
                               row * session.encoder().configuration().height,
                               column * session.encoder().configuration().width);
            });
          }

          pool.waitAll();
        }
      }

      return NV_ENC_SUCCESS;
    }

    const std::vector<std::shared_ptr<VideoEncoder>> &encoders() const { return encoders_; }
    EncodeAPI& api() { return encoders()[0]->api(); } //TODO
    size_t rows() const { return rows_; }
    size_t columns() const { return columns_; }

private:
    VideoLock lock_;
    const EncodeConfiguration encoderConfiguration_;
    CUVIDFrameQueue frameQueue_;
    std::vector<std::shared_ptr<VideoEncoder>> encoders_;
    CudaDecoder decoder_; //TODO this should be a base VideoDecoder
    const size_t rows_, columns_;
    GPUThreadPool pool;

    static const std::vector<std::shared_ptr<VideoEncoder>> CreateEncoders(
            GPUContext& context, const EncodeConfiguration& configuration, VideoLock &lock, size_t count) {
      std::vector<std::shared_ptr<VideoEncoder>> encoders;

      encoders.reserve(count);
      std::generate_n(std::back_inserter(encoders), count,
                      [&context, &configuration, &lock]() {
                          return std::make_shared<VideoEncoder>(context, configuration, lock); });

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
