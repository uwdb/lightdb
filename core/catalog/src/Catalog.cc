#include "Catalog.h"
#include "LightField.h"
#include "PhysicalOperators.h"
#include "Model.h"
#include "Transaction.h"
#include "Gpac.h"

namespace lightdb::catalog {
    std::optional<Catalog> Catalog::instance_;

    bool Catalog::catalog_exists(const std::filesystem::path &path) {
        return std::filesystem::exists(path) &&
               std::filesystem::is_directory(path);
    }

    bool Catalog::exists(const std::string &name) const {
        auto path = std::filesystem::absolute(path_ / name);
        return std::filesystem::exists(path) &&
               std::filesystem::is_directory(path);
    }

    Entry Catalog::entry(const std::string &name) const {
        auto path = std::filesystem::absolute(path_ / name);
        return Entry{*this, name, path};
    }

    unsigned int Entry::load_version(const std::filesystem::path &path) {
        auto filename = Files::version_filename(path);

        if(std::filesystem::exists(filename)) {
            std::ifstream f(filename);
            return static_cast<unsigned int>(stoul(std::string(std::istreambuf_iterator<char>(f),
                                                   std::istreambuf_iterator<char>())));
        } else
            return 0u;
    }

    unsigned int Entry::write_version(const std::filesystem::path &path, const unsigned int version) {
        auto string = std::to_string(version);

        std::ofstream output(catalog::Files::version_filename(path));
        std::copy(string.begin(), string.end(), std::ostreambuf_iterator<char>(output));

        return version;
    }

    unsigned int Entry::increment_version(const std::filesystem::path &path) {
        return write_version(path, load_version(path) + 1);
    }


    std::vector<Source> Entry::load_sources() {
        auto filename = Files::metadata_filename(path(), version());

        return std::filesystem::exists(filename)
            ? video::gpac::load_metadata(filename)
            : std::vector<Source>{};
    }

    LightFieldReference Catalog::get(const std::string &name, const bool create) const {
        if(exists(name))
            return LightFieldReference::make<logical::ScannedLightField>(entry(name));
        else if(create)
            return this->create(name);
        else
            throw CatalogError(std::string("Light field '") + name + "' does not exist in catalog " + path_.string(), name);
    }

    LightFieldReference Catalog::create(const std::string& name) const {
        auto path = std::filesystem::absolute(path_ / name);

        if(!std::filesystem::exists(path))
            std::filesystem::create_directory(path);
        else
            throw CatalogError("Entry already exists", name);

        return LightFieldReference::make<logical::ScannedLightField>(Entry{*this, name, path});
    }

    std::vector<Source> ExternalEntry::load_sources(const std::optional<Volume>& volume,
                                                    const std::optional<GeometryReference>& geometry) {
        if(video::gpac::can_mux(filename()))
            return video::gpac::load_metadata(filename(), false, volume, geometry);
        else if(!volume.has_value())
            throw CatalogError("Volume must be supplied for external files with no metadata", filename());
        else if(!geometry.has_value())
            throw CatalogError("Geometry must be supplied for external files with no metadata", filename());
        else
            return functional::transform<Source>(video::ffmpeg::GetStreamConfigurations(filename(), true),
                    [this, &volume, &geometry](const auto &configuration) {
                        return Source{
                            0, filename(), configuration.decode.codec, configuration.decode,
                            volume.value() | TemporalRange{volume->t().start(),
                                                           volume->t().start() + configuration.duration},
                            geometry.value()}; });
    }
} // namespace lightdb::catalog