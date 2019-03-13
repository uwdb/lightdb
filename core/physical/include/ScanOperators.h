#ifndef LIGHTDB_SCANOPERATORS_H
#define LIGHTDB_SCANOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class ScanSingleFileDecodeReader: public FrameLightField, public EncodedVideoInterface {
public:
    explicit ScanSingleFileDecodeReader(const LightFieldReference &logical, catalog::Stream stream)
            : FrameLightField(logical, DeviceType::CPU, runtime::make<Runtime>(*this)),
              stream_(std::move(stream))
    { }

    const catalog::Stream &stream() const { return stream_; }
    const Codec &codec() const override { return stream_.codec(); }

private:
    class Runtime: public FrameLightField::Runtime<ScanSingleFileDecodeReader> {
    public:
        explicit Runtime(ScanSingleFileDecodeReader &physical)
            : FrameLightField::Runtime<ScanSingleFileDecodeReader>(physical),
              reader_(physical.stream().path())
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            auto packet = reader_.read();
            return packet.has_value()
                   ? std::optional<physical::MaterializedLightFieldReference>{
                            CPUEncodedFrameData(physical().stream().codec(), physical().stream().configuration(), packet.value())}
                   : std::nullopt;
        }

        FileDecodeReader reader_;
    };

    const catalog::Stream stream_;
};

template<size_t Size=131072>
class ScanSingleFile: public FrameLightField, public EncodedVideoInterface {
public:
    explicit ScanSingleFile(const LightFieldReference &logical, catalog::Stream stream)
            : FrameLightField(logical, DeviceType::CPU, runtime::make<Runtime>(*this)),
              stream_(std::move(stream))
    { }

    const catalog::Stream &stream() const { return stream_; }
    const Codec &codec() const override { return stream_.codec(); }

private:
    class Runtime: public FrameLightField::Runtime<ScanSingleFile<Size>> {
    public:
        explicit Runtime(ScanSingleFile &physical)
            : FrameLightField::Runtime<ScanSingleFile<Size>>(physical),
              buffer_(Size, 0),
              reader_(physical.stream().path())
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(!reader_.eof()) {
                reader_.read(buffer_.data(), buffer_.size());
                return {CPUEncodedFrameData(
                        this->physical().stream().codec(),
                        this->physical().stream().configuration(),
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

    const catalog::Stream stream_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_SCANOPERATORS_H
