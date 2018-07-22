#ifndef LIGHTDB_DECODEOPERATORS_H
#define LIGHTDB_DECODEOPERATORS_H

#include "LightField.h"
#include "Environment.h"
#include "PhysicalOperators.h"
#include "Data.h"
#include "VideoDecoderSession.h"

namespace lightdb::physical {

class GPUDecode : public GPUUnaryOperator<CPUEncodedFrameData> {
public:
    explicit GPUDecode(const LightFieldReference &logical,
                       PhysicalLightFieldReference source,
                       const execution::GPU &gpu)
            : GPUDecode(logical, source, source.expect_downcast<EncodedVideoInterface>(), gpu)
    { }

    GPUDecode(const GPUDecode &) = delete;
    GPUDecode(GPUDecode &&) = default;

    std::optional<physical::DataReference> read() override {
        std::vector<GPUFrameReference> frames;

        LOG_IF(WARNING, decode_configuration_->output_surfaces < 8)
            << "Decode configuration output surfaces is low, limiting throughput";

        if(!decoder_->frame_queue().isComplete())
            do
                frames.emplace_back(session_->decode());
            while(!decoder_->frame_queue().isEmpty() &&
                  !decoder_->frame_queue().isEndOfDecode() &&
                    frames.size() <= decode_configuration_->output_surfaces / 4);

        if(!frames.empty() || !decoder_->frame_queue().isComplete())
            return std::optional<physical::DataReference>{GPUDecodedFrameData(frames)};
        else
            return std::nullopt;
    }

protected:
    explicit GPUDecode(const LightFieldReference &logical,
                       const PhysicalLightFieldReference &source,
                       EncodedVideoInterface &encoded,
                       const execution::GPU &gpu)
            : GPUUnaryOperator(logical, source, gpu, [&encoded](){return encoded.configuration(); }),
              decode_configuration_([&encoded, this]() {
                  return DecodeConfiguration{configuration(), encoded.codec().cudaId().value()}; }),
              queue_([this]() { return CUVIDFrameQueue(lock()); }),
              decoder_([this]() { return CudaDecoder(decode_configuration_, queue_, lock()); }),
              session_([this]() {
                  return VideoDecoderSession<downcast_iterator<CPUEncodedFrameData>>(
                          decoder_, iterator(), iterator().eos()); }) {
        CHECK_EQ(source->device(), DeviceType::GPU);
    }

private:
    lazy<DecodeConfiguration> decode_configuration_;
    lazy<CUVIDFrameQueue> queue_;
    lazy<CudaDecoder> decoder_;
    lazy<VideoDecoderSession<downcast_iterator<CPUEncodedFrameData>>> session_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_DECODEOPERATORS_H
