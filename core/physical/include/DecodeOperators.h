#ifndef LIGHTDB_DECODEOPERATORS_H
#define LIGHTDB_DECODEOPERATORS_H

#include "LightField.h"
#include "Environment.h"
#include "PhysicalOperators.h"
#include "MaterializedLightField.h"
#include "VideoDecoderSession.h"

namespace lightdb::physical {

class GPUDecode : public GPUUnaryOperator<CPUEncodedFrameData> {
public:
    explicit GPUDecode(const LightFieldReference &logical,
                       PhysicalLightFieldReference source,
                       const execution::GPU &gpu)
            : GPUDecode(logical,
                        source,
                        source.expect_downcast<EncodedVideoInterface>(),
                        gpu,
                        std::chrono::milliseconds(1u))
    { }

    template<typename Rep, typename Period>
    explicit GPUDecode(const LightFieldReference &logical,
                       PhysicalLightFieldReference source,
                       const execution::GPU &gpu,
                       const std::chrono::duration<Rep, Period> &poll_duration)
            : GPUDecode(logical,
                        source,
                        source.expect_downcast<EncodedVideoInterface>(),
                        gpu,
                        poll_duration)
    { }

    GPUDecode(const GPUDecode &) = delete;
    GPUDecode(GPUDecode &&) = default;

    std::optional<physical::MaterializedLightFieldReference> read() override {
        std::vector<GPUFrameReference> frames;

        LOG_IF(WARNING, decode_configuration_->output_surfaces < 8)
            << "Decode configuration output surfaces is low, limiting throughput";

        if(!decoder_->frame_queue().isComplete())
            do {
                auto frame = session_->decode(poll_duration_);
                if (frame.has_value())
                    frames.emplace_back(frame.value());
            } while(!decoder_->frame_queue().isEmpty() &&
                    !decoder_->frame_queue().isEndOfDecode() &&
                    frames.size() <= decode_configuration_->output_surfaces / 4);

        if(!frames.empty() || !decoder_->frame_queue().isComplete())
            return std::optional<physical::MaterializedLightFieldReference>{GPUDecodedFrameData(frames)};
        else
            return std::nullopt;
    }

protected:
    template<typename Rep, typename Period>
    explicit GPUDecode(const LightFieldReference &logical,
                       const PhysicalLightFieldReference &source,
                       EncodedVideoInterface &encoded,
                       const execution::GPU &gpu,
                       std::chrono::duration<Rep, Period> poll_duration)
            : GPUUnaryOperator(logical, source, gpu, [&encoded](){return encoded.configuration(); }),
              poll_duration_(poll_duration),
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
    const std::chrono::microseconds poll_duration_;
    lazy<DecodeConfiguration> decode_configuration_;
    lazy<CUVIDFrameQueue> queue_;
    lazy<CudaDecoder> decoder_;
    lazy<VideoDecoderSession<downcast_iterator<CPUEncodedFrameData>>> session_;
};

template<typename T>
class CPUFixedLengthRecordDecode : public PhysicalLightField {
public:
    CPUFixedLengthRecordDecode(const LightFieldReference &logical,
                               const PhysicalLightFieldReference &source)
            : PhysicalLightField(logical, {source}, DeviceType::CPU) {
        CHECK_EQ(source->device(), DeviceType::CPU);
    }

    CPUFixedLengthRecordDecode(const CPUFixedLengthRecordDecode &) = delete;
    CPUFixedLengthRecordDecode(CPUFixedLengthRecordDecode&&) noexcept = default;

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(iterators()[0] != iterators()[0].eos()) {
            CPUDecodedFrameData output;
            auto input = iterators()[0]++;
            auto &data = input.downcast<CPUEncodedFrameData>();

            for(auto *current = data.value().data(),
                     *end = current + data.value().size();
                  current < end;
                  current += sizeof(T))
                output.frames().emplace_back(LocalFrameReference::make<LocalFrame>(0u, 0u, lightdb::bytestring(current, current + sizeof(T))));

            return output;
        } else
            return {};
    }
};

} // namespace lightdb::physical

#endif //LIGHTDB_DECODEOPERATORS_H
