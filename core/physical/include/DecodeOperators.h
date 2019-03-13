#ifndef LIGHTDB_DECODEOPERATORS_H
#define LIGHTDB_DECODEOPERATORS_H

#include "LightField.h"
#include "Environment.h"
#include "PhysicalOperators.h"
#include "MaterializedLightField.h"
#include "VideoDecoderSession.h"
#include "Runtime.h"

namespace lightdb::physical {

class GPUDecodeFromCPU : public GPUUnaryOperator {
public:
    explicit GPUDecodeFromCPU(const LightFieldReference &logical,
                              PhysicalLightFieldReference source,
                              const execution::GPU &gpu)
            : GPUDecodeFromCPU(logical,
                               source,
                               source.expect_downcast<EncodedVideoInterface>(),
                               gpu,
                               std::chrono::milliseconds(1u))
    { }

    template<typename Rep, typename Period>
    explicit GPUDecodeFromCPU(const LightFieldReference &logical,
                              PhysicalLightFieldReference source,
                              const execution::GPU &gpu,
                              const std::chrono::duration<Rep, Period> &poll_duration)
            : GPUDecodeFromCPU(logical,
                               source,
                               source.expect_downcast<EncodedVideoInterface>(),
                               gpu,
                               poll_duration)
    { }

    GPUDecodeFromCPU(const GPUDecodeFromCPU&) = delete;
    GPUDecodeFromCPU(GPUDecodeFromCPU&&) = default;

    std::chrono::microseconds poll_duration() const { return poll_duration_; }

protected:
    template<typename Rep, typename Period>
    explicit GPUDecodeFromCPU(const LightFieldReference &logical,
                              const PhysicalLightFieldReference &source,
                              EncodedVideoInterface &encoded,
                              const execution::GPU &gpu,
                              std::chrono::duration<Rep, Period> poll_duration)
            : GPUUnaryOperator(logical, source, runtime::make<Runtime>(*this, encoded), gpu), //, [&encoded](){return encoded.configuration(); }),
              poll_duration_(poll_duration) {
        CHECK_EQ(source->device(), DeviceType::CPU);
    }

private:
    class Runtime: public GPUUnaryOperator::Runtime<GPUDecodeFromCPU, CPUEncodedFrameData> {
    public:
        explicit Runtime(GPUDecodeFromCPU &physical, EncodedVideoInterface &encoded)
            : GPUUnaryOperator::Runtime<GPUDecodeFromCPU, CPUEncodedFrameData>(physical),
              decode_configuration_{/*encoded.*/ configuration(), encoded.codec()},
              queue_{lock()},
              decoder_{decode_configuration_, queue_, lock()},
              session_{decoder_, iterator(), iterator().eos()}
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            std::vector<GPUFrameReference> frames;

            LOG_IF(WARNING, decode_configuration_.output_surfaces < 8)
                << "Decode configuration output surfaces is low, limiting throughput";

            if(!decoder_.frame_queue().isComplete())
                do {
                    auto frame = session_.decode(physical().poll_duration());
                    if (frame.has_value())
                        frames.emplace_back(frame.value());
                } while(!decoder_.frame_queue().isEmpty() &&
                        !decoder_.frame_queue().isEndOfDecode() &&
                        frames.size() <= decode_configuration_.output_surfaces / 4);

            if(!frames.empty() || !decoder_.frame_queue().isComplete())
                return std::optional<physical::MaterializedLightFieldReference>{
                          GPUDecodedFrameData(decode_configuration_, frames)};
            else
                return std::nullopt;
        }

    private:
        DecodeConfiguration decode_configuration_;
        CUVIDFrameQueue queue_;
        CudaDecoder decoder_;
        VideoDecoderSession<Runtime::downcast_iterator<CPUEncodedFrameData>> session_;
    };

    const std::chrono::microseconds poll_duration_;
};

template<typename T>
class CPUFixedLengthRecordDecode : public PhysicalLightField {
public:
    CPUFixedLengthRecordDecode(const LightFieldReference &logical,
                               const PhysicalLightFieldReference &source)
            : PhysicalLightField(logical, {source}, DeviceType::CPU, runtime::make<Runtime>(*this)) {
        CHECK_EQ(source->device(), DeviceType::CPU);
    }

    CPUFixedLengthRecordDecode(const CPUFixedLengthRecordDecode &) = delete;
    CPUFixedLengthRecordDecode(CPUFixedLengthRecordDecode&&) noexcept = default;

private:
    class Runtime: public runtime::UnaryRuntime<CPUFixedLengthRecordDecode, CPUEncodedFrameData> {
    public:
        explicit Runtime(CPUFixedLengthRecordDecode &physical)
            : runtime::UnaryRuntime<CPUFixedLengthRecordDecode, CPUEncodedFrameData>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(this->iterator() != this->iterator().eos()) {
            //if(iterators()[0] != iterators()[0].eos()) {
                //auto input = iterators()[0]++;
                //auto &data = input.downcast<CPUEncodedFrameData>();
                auto data = this->iterator()++; //input.downcast<CPUEncodedFrameData>();
                CPUDecodedFrameData output{data.configuration()};

                for(auto *current = data.value().data(),
                            *end = current + data.value().size();
                    current < end;
                    current += sizeof(T))
                    output.frames().emplace_back(LocalFrameReference::make<LocalFrame>(
                            0u, 0u, lightdb::bytestring(current, current + sizeof(T))));

                return output;
            } else
                return {};
        }
    };
};

} // namespace lightdb::physical

#endif //LIGHTDB_DECODEOPERATORS_H
