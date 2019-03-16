#ifndef LIGHTDB_DECODEOPERATORS_H
#define LIGHTDB_DECODEOPERATORS_H

#include "LightField.h"
#include "Environment.h"
#include "PhysicalOperators.h"
#include "MaterializedLightField.h"
#include "VideoDecoderSession.h"
#include "Runtime.h"

namespace lightdb::physical {

class GPUDecodeFromCPU : public PhysicalOperator, public GPUOperator {
public:
    explicit GPUDecodeFromCPU(const LightFieldReference &logical,
                              PhysicalOperatorReference source,
                              const execution::GPU &gpu)
            : GPUDecodeFromCPU(logical,
                               source,
                               gpu,
                               std::chrono::milliseconds(1u))
    { }

    template<typename Rep, typename Period>
    explicit GPUDecodeFromCPU(const LightFieldReference &logical,
                              PhysicalOperatorReference &source,
                              const execution::GPU &gpu,
                              std::chrono::duration<Rep, Period> poll_duration)
            : PhysicalOperator(logical, {source}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(gpu),
              poll_duration_(poll_duration) {
        CHECK_EQ(source->device(), DeviceType::CPU);
    }

    GPUDecodeFromCPU(const GPUDecodeFromCPU&) = delete;
    GPUDecodeFromCPU(GPUDecodeFromCPU&&) = default;

    std::chrono::microseconds poll_duration() const { return poll_duration_; }

private:
    class Runtime: public runtime::GPUUnaryRuntime<GPUDecodeFromCPU, CPUEncodedFrameData> {
    public:
        explicit Runtime(GPUDecodeFromCPU &physical)
            : runtime::GPUUnaryRuntime<GPUDecodeFromCPU, CPUEncodedFrameData>(physical),
              decode_configuration_{configuration(), codec()},
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
class CPUFixedLengthRecordDecode : public PhysicalOperator {
public:
    CPUFixedLengthRecordDecode(const LightFieldReference &logical,
                               const PhysicalOperatorReference &source)
            : PhysicalOperator(logical, {source}, DeviceType::CPU, runtime::make<Runtime>(*this)) {
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
                auto data = this->iterator()++;
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
