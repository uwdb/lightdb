#ifndef LIGHTDB_SCANOPERATORS_H
#define LIGHTDB_SCANOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class ScanSingleFileDecodeReader: public PhysicalLightField, public EncodedVideoInterface {
public:
    explicit ScanSingleFileDecodeReader(const LightFieldReference &logical, catalog::Stream stream)
            : PhysicalLightField(logical, DeviceType::CPU),
              stream_(std::move(stream)),
              reader_([this]() { return FileDecodeReader(stream_.path()); })
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        auto packet = reader_->read();
        return packet.has_value()
               ? std::optional<physical::MaterializedLightFieldReference>{
                    CPUEncodedFrameData(codec(), configuration(), packet.value())}
               : std::nullopt;
    }

    const Codec &codec() const override { return stream_.codec(); }
    const Configuration &configuration() override { return stream_.configuration(); }

private:
    const catalog::Stream stream_;
    lazy<FileDecodeReader> reader_;
};

template<size_t Size=131072>
class ScanSingleFile: public PhysicalLightField, public EncodedVideoInterface {
public:
    explicit ScanSingleFile(const LightFieldReference &logical, catalog::Stream stream)
            : PhysicalLightField(logical, DeviceType::CPU),
              stream_(std::move(stream)),
              buffer_(Size, 0),
              reader_([this]() { return std::ifstream(stream_.path()); })
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(!reader_->eof()) {
            reader_->read(buffer_.data(), buffer_.size());
            return {CPUEncodedFrameData(stream_.codec(), configuration(), buffer_)};

        } else {
            reader_->close();
            return {};
        }
    }

    const Codec &codec() const override { return stream_.codec(); }
    const Configuration &configuration() override { return stream_.configuration(); }

private:
    const catalog::Stream stream_;
    lightdb::bytestring buffer_;
    lazy<std::ifstream> reader_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_SCANOPERATORS_H
