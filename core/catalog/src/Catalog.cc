#include "Catalog.h"
#include "LightField.h"
#include "errors.h"

namespace lightdb::catalog {

    LightFieldReference Catalog::get(const std::string &name) const
    {
        auto metadataFile = path_ / name;

        if(!std::experimental::filesystem::exists(metadataFile))
            throw CatalogError("Catalog does not exist", name);
        else
            return LightFieldReference::make<logical::ScannedLightField>(Metadata{*this, metadataFile});
    }

} // namespace lightdb::catalog