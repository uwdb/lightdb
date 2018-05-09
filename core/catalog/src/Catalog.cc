#include "Catalog.h"
#include "LightField.h"
#include <glog/logging.h>

namespace filesystem = ::std::experimental::filesystem;

namespace lightdb::catalog {
    std::optional<Catalog> Catalog::instance_;

    LightFieldReference Catalog::get(const std::string &name) const
    {
        auto metadataFilename = filesystem::absolute(path_ / name / metadataFilename_);

        if(!filesystem::exists(metadataFilename))
            throw CatalogError("Light field does not exist", name);
        else
            return LightFieldReference::make<logical::ScannedLightField>(Metadata{*this, name, metadataFilename});
    }

    const std::vector<std::string> Catalog::Metadata::streams() const noexcept {
        LOG(WARNING) << "Using unimplemented stub Catalog::streams";

        return {filesystem::path(path_).replace_filename("stream0.h264")};
    }

} // namespace lightdb::catalog