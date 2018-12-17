#ifndef LIGHTDB_SCANOPERATORS_H
#define LIGHTDB_SCANOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class ScanSingleFileToGPU: public PhysicalLightField, public EncodedVideoInterface {
public:
    explicit ScanSingleFileToGPU(const LightFieldReference &logical, const catalog::Stream &stream)
            : ScanSingleFileToGPU(logical,
                                  logical->downcast<logical::ScannedLightField>(),
                                  stream)
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        auto packet = reader_->read();
        return packet.has_value()
               ? std::optional<physical::MaterializedLightFieldReference>{CPUEncodedFrameData(codec(), packet.value())}
               : std::nullopt;
    }

    const Codec &codec() const override { return stream_.codec(); }
    const Configuration &configuration() override { return stream_.configuration(); }

private:
    explicit ScanSingleFileToGPU(const LightFieldReference &logical,
                                 const logical::ScannedLightField &scanned,
                                 catalog::Stream stream)
            : PhysicalLightField(logical, DeviceType::GPU),
              stream_(std::move(stream)),
              scanned_(scanned),
              reader_([this]() { return FileDecodeReader(stream_.path()); })
    { }

    const catalog::Stream stream_;
    const logical::ScannedLightField &scanned_;
    lazy<FileDecodeReader> reader_;
};

template<size_t Size=131072>
class ScanSingleFile: public PhysicalLightField, public EncodedVideoInterface {
public:
    explicit ScanSingleFile(const LightFieldReference &logical, const catalog::Stream &stream)
            : ScanSingleFile(logical,
                             logical->downcast<logical::ScannedLightField>(),
                             stream)
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(!reader_->eof()) {
            reader_->read(buffer_.data(), buffer_.size());
            return {CPUEncodedFrameData(stream_.codec(), buffer_)};

        } else {
            reader_->close();
            return {};
        }
    }

    const Codec &codec() const override { return stream_.codec(); }
    const Configuration &configuration() override { return stream_.configuration(); }

private:
    explicit ScanSingleFile(const LightFieldReference &logical,
                            const logical::ScannedLightField &scanned,
                            catalog::Stream stream)
            : PhysicalLightField(logical, DeviceType::CPU),
              stream_(std::move(stream)),
              scanned_(scanned),
              buffer_(Size, 0),
              reader_([this]() { return std::ifstream(stream_.path()); })
    { }

    const catalog::Stream stream_;
    const logical::ScannedLightField &scanned_;
    lightdb::bytestring buffer_;
    lazy<std::ifstream> reader_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_SCANOPERATORS_H
