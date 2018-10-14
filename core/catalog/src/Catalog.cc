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

    OutputStream Catalog::create(const std::string& name, const Codec &codec, const Configuration &configuration) const {
        auto path = filesystem::absolute(path_ / name);
        auto metadataFilename = path / metadataFilename_;
        auto streamFilename = path / "stream0.hevc";

        if(!filesystem::exists(path))
            filesystem::create_directory(path);

        LOG(INFO) << "Assuming TLF version 0";
        LOG(ERROR) << "Not creating catalog metadata";
        return OutputStream(streamFilename, codec, configuration);
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