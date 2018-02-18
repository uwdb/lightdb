#include "Catalog.h"
#include "LightField.h"

namespace filesystem = ::std::experimental::filesystem;

namespace lightdb::catalog {
    std::optional<Catalog> Catalog::instance_;

    LightFieldReference Catalog::get(const std::string &name) const
    {
        auto metadataFilename = filesystem::absolute(path_ / name / metadataFilename_);

        if(!filesystem::exists(metadataFilename))
            throw CatalogError("Light field does not exist", name);
        else
            return LightFieldReference::make<logical::ScannedLightField>(Metadata{*this, metadataFilename});
    }

} // namespace lightdb::catalog