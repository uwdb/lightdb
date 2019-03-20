#include "Catalog.h"
#include "LightField.h"
#include "PhysicalOperators.h"
#include "Model.h"
#include "Gpac.h"

namespace lightdb::catalog {
    std::optional<Catalog> Catalog::instance_;

    bool Catalog::catalog_exists(const std::filesystem::path &path) {
        return std::filesystem::exists(path) &&
               std::filesystem::is_directory(path);
    }

    bool Catalog::exists(const std::string &name) const {
        auto path = std::filesystem::absolute(path_ / name);
        auto metadataFilename = path / metadataFilename_;
        return std::filesystem::exists(metadataFilename) &&
               std::filesystem::is_regular_file(metadataFilename);
    }

    Catalog::Metadata Catalog::metadata(const std::string &name) const {
        auto path = std::filesystem::absolute(path_ / name);
        return Catalog::Metadata{*this, name, path};
    }

    LightFieldReference Catalog::get(const std::string &name) const {
        if(!exists(name))
            throw CatalogError(std::string("Light field ") + name + " does not exist in catalog " + path_.string(), name);
        else
            return LightFieldReference::make<logical::ScannedLightField>(metadata(name));
    }

    OutputStream Catalog::create(const std::string& name, const Codec &codec, const Configuration &configuration) const {
        auto path = std::filesystem::absolute(path_ / name);
        auto metadataFilename = path / metadataFilename_;
        auto streamFilename = path / "stream0.hevc";

        if(!std::filesystem::exists(path))
            std::filesystem::create_directory(path);

        LOG(INFO) << "Assuming TLF version 0";
        LOG(WARNING) << "Not creating catalog metadata";
        return OutputStream(streamFilename, codec, configuration);
    }

    std::vector<Stream> Catalog::get_streams(const std::filesystem::path &path) {
        return video::gpac::GetStreams(path / metadataFilename_);
    }

} // namespace lightdb::catalog