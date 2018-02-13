#include "Catalog.h"
#include "LightField.h"

namespace lightdb::catalog {

    LightFieldReference Catalog::get(const std::string &name) const
    {
        auto metadataFile = path_ / name;

        if(!std::experimental::filesystem::exists(metadataFile))
            throw std::runtime_error("catalog file does not exist");
        else
            return LightFieldReference::make<logical::ScannedLightField>(Metadata{*this, metadataFile});
    }

} // namespace lightdb::catalog