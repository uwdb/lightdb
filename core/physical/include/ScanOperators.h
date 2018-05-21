#ifndef LIGHTDB_SCANOPERATORS_H
#define LIGHTDB_SCANOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class ScanSingleFile: public PhysicalLightField, public EncodedVideoInterface {
public:
    explicit ScanSingleFile(const LightFieldReference &logical, const catalog::Stream &stream)
            : ScanSingleFile(logical, logical->downcast<logical::ScannedLightField>(), stream)
    { }

    std::optional<physical::DataReference> read() override {
        auto packet = reader_->read();
        return packet.has_value()
               ? std::optional<physical::DataReference>{CPUEncodedFrameData(codec(), packet.value())}
               : std::nullopt;
    }

    const Codec &codec() const override { return stream_.codec(); }
    const Configuration &configuration() const override { return stream_.configuration(); }

private:
    explicit ScanSingleFile(const LightFieldReference &logical,
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

} // namespace lightdb::physical

#endif //LIGHTDB_SCANOPERATORS_H
