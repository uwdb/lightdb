#include "Catalog.h"
#include "LightField.h"
#include "PhysicalOperators.h"
#include "Model.h"

namespace filesystem = ::std::experimental::filesystem;

namespace lightdb::catalog {
    std::optional<Catalog> Catalog::instance_;

    LightFieldReference Catalog::get(const std::string &name) const {
        auto path = filesystem::absolute(path_ / name);
        auto metadataFilename = path / metadataFilename_;

        if(!filesystem::exists(metadataFilename))
            throw CatalogError("Light field does not exist", name);
        else
            return LightFieldReference::make<logical::ScannedLightField>(Metadata{*this, name, path});
    }

    void Catalog::save(const std::string& name, PhysicalLightField& sink) const {
        auto path = filesystem::absolute(path_ / name);
        auto metadataFilename = path / metadataFilename_;
        auto streamFilename = path / "stream0.hevc";

        LOG(INFO) << "Assuming version 0";
        if(!filesystem::exists(path))
            filesystem::create_directory(path);

        std::ofstream file(streamFilename);
        std::optional<physical::MaterializedLightFieldReference> packet;

        while((packet = sink.read()).has_value()) {
            auto &encoded = packet.value().downcast<physical::CPUEncodedFrameData>();
            std::copy(encoded.value().begin(),encoded.value().end(),std::ostreambuf_iterator<char>(file));
        }

        LOG(INFO) << "Creating metadata symlink instead of doing it correctly";
        symlink(streamFilename.c_str(), metadataFilename.c_str());
    }


    std::vector<Stream> Catalog::get_streams(const filesystem::path &path) {
        LOG(WARNING) << "Using unimplemented stub Catalog::get_streams";
        LOG(WARNING) << "Using hardcoded configuration";
        LOG(WARNING) << "Using hardcoded first stream for video configuration";

        const size_t index = 0;
        auto metadata = utility::StreamMetadata(path / metadataFilename_, index, false);
        auto hardcoded_hevc_path = filesystem::absolute(path / "stream0.hevc");
        auto hardcoded_h264_path = filesystem::absolute(path / "stream0.h264");
        return {Stream{filesystem::exists(hardcoded_hevc_path) ? hardcoded_hevc_path : hardcoded_h264_path, metadata.codec(), metadata.configuration()}};
    }

} // namespace lightdb::catalog