#include "Catalog.h"
#include "LightField.h"
#include <glog/logging.h>

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
        return {Stream{filesystem::absolute(path / "stream0.h264"),
                       Codec::h264(),
                       Configuration{320, 240, 0, 0, 1024*1024, {24, 1}}}};
    }

} // namespace lightdb::catalog