#include "Catalog.h"
#include "LightField.h"
#include "Model.h"
#include "Ffmpeg.h"

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

    std::vector<Stream> Catalog::get_streams(const filesystem::path &path) {
        LOG(WARNING) << "Using unimplemented stub Catalog::get_streams";
        LOG(WARNING) << "Using hardcoded configuration";
        LOG(WARNING) << "Using hardcoded first stream for video configuration";

        const size_t index = 0;
        auto metadata = utility::StreamMetadata(path / metadataFilename_, index, false);
        return {Stream{filesystem::absolute(path / "stream0.h264"), metadata.codec(), metadata.configuration()}};
    }

} // namespace lightdb::catalog