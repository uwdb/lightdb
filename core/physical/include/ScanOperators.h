#ifndef LIGHTDB_SCANOPERATORS_H
#define LIGHTDB_SCANOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class ScanSingleFileDecodeReader: public PhysicalOperator {
public:
    explicit ScanSingleFileDecodeReader(const LightFieldReference &logical, catalog::Source source)
            : PhysicalOperator(logical, DeviceType::CPU, runtime::make<Runtime>(*this)),
              source_(std::move(source))
    { }

    const catalog::Source &source() const { return source_; }
    const Codec &codec() const { return source_.codec(); }

private:
    class Runtime: public runtime::Runtime<ScanSingleFileDecodeReader> {
    public:
        explicit Runtime(ScanSingleFileDecodeReader &physical)
            : runtime::Runtime<ScanSingleFileDecodeReader>(physical),
              reader_(physical.context()->environment().gpus().empty()
                          ? static_cast<std::unique_ptr<video::DecodeReader>>(
                                  std::make_unique<video::FfmpegFileDecodeReader>(physical.source().filename()))
                          : std::make_unique<video::FileDecodeReader>(physical.source().filename()))
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            auto packet = reader_->read();
            return packet.has_value()
                   ? std::optional<physical::MaterializedLightFieldReference>{
                            physical::MaterializedLightFieldReference::make<CPUEncodedFrameData>(
                                    physical().source().codec(),
                                    physical().source().configuration(),
                                    physical().source().geometry(),
                                    packet.value())}
                   : std::nullopt;
        }

    private:
        std::unique_ptr<video::DecodeReader> reader_;
    };

    const catalog::Source source_;
};

template<size_t Size=131072>
class ScanSingleFile: public PhysicalOperator {
public:
    explicit ScanSingleFile(const LightFieldReference &logical, catalog::Source source)
            : PhysicalOperator(logical, DeviceType::CPU, runtime::make<Runtime>(*this)),
              source_(std::move(source))
    { }

    const catalog::Source &source() const { return source_; }
    const Codec &codec() const { return source_.codec(); }

private:
    class Runtime: public runtime::Runtime<ScanSingleFile<Size>> {
    public:
        explicit Runtime(ScanSingleFile &physical)
            : runtime::Runtime<ScanSingleFile<Size>>(physical),
              buffer_(Size, 0),
              reader_(physical.source().filename())
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(!reader_.eof()) {
                reader_.read(buffer_.data(), buffer_.size());
                return {CPUEncodedFrameData(
                        this->physical().source().codec(),
                        this->physical().source().configuration(),
                        this->physical().source().geometry(),
                        buffer_)};

            } else {
                reader_.close();
                return {};
            }
        }

    private:
        lightdb::bytestring buffer_;
        std::ifstream reader_;
    };

    const catalog::Source source_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_SCANOPERATORS_H
