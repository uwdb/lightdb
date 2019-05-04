#ifndef LIGHTDB_CATALOG_H
#define LIGHTDB_CATALOG_H

#include "Codec.h"
#include "Geometry.h"
#include "Color.h"
#include "Configuration.h"
#include <filesystem>
#include <utility>
#include <fstream>

namespace lightdb {
    class LightField;
    class PhysicalOperator;
    namespace logical { class Algebra; }
    using LightFieldReference = shared_reference<LightField, logical::Algebra>;

    namespace catalog {
        class Entry;

        class Source {
        public:
            Source(const unsigned int index, std::filesystem::path filename,
                   Codec codec, const Configuration &configuration,
                   CompositeVolume volume, const GeometryReference &geometry)
                    : index_(index),
                      filename_(std::move(filename)),
                      codec_(std::move(codec)),
                      configuration_(configuration),
                      volume_(std::move(volume)),
                      geometry_(geometry)
            { }

            unsigned int index() const { return index_; }
            const std::filesystem::path& filename() const { return filename_; }
            const Codec& codec() const { return codec_; }
            const CompositeVolume volume() const { return volume_; }
            const Configuration& configuration() const { return configuration_; }
            const GeometryReference &geometry() const { return geometry_; }

        private:
            const unsigned int index_;
            const std::filesystem::path filename_;
            const Codec codec_;
            const Configuration configuration_;
            const CompositeVolume volume_;
            const GeometryReference geometry_;
        };

        class Catalog {
        public:
            Catalog(const Catalog &catalog) = default;
            explicit Catalog(std::filesystem::path path)
                    : path_(std::move(asserts::CHECK_NONEMPTY(path)))
            { }

            static const Catalog &instance()
            {
                if(instance_.has_value())
                    return instance_.value();
                else
                    throw CatalogError("No ambient catalog specified", "instance");
            }
            static const Catalog &instance(Catalog catalog) { return instance_.emplace(catalog); }
            static bool catalog_exists(const std::filesystem::path &path);

            LightFieldReference get(const std::string &name, bool create=false) const;
            LightFieldReference create(const std::string& name) const;
            bool exists(const std::string &name) const;
            const std::filesystem::path &path() const { return path_; }

        private:
            Catalog() : path_("") { }

            const std::filesystem::path path_;

            inline Entry entry(const std::string &name) const;

            static std::optional<Catalog> instance_;
        };

        class Entry {
            friend class Catalog;

            Entry(const Catalog &catalog, std::string name, std::filesystem::path path)
                    : catalog_(catalog),
                      name_(std::move(name)),
                      path_(std::move(path)),
                    //TODO grab boxes and load volume/geometry, detect colorspace
                      volume_({{0, 0}, {0, 0}, {0, 0}, {0, 10}, ThetaRange::limits(), PhiRange::limits()}),
                      colorSpace_(YUVColorSpace::instance()),
                    //TODO move geometry to streams
                      geometry_(EquirectangularGeometry(EquirectangularGeometry::Samples())),
                      version_(load_version(path_)),
                      sources_(load_sources())
            { CHECK(std::filesystem::exists(path_)); }

        public:
            inline const std::string& name() const noexcept { return name_; }
            inline const Catalog& catalog() const noexcept { return catalog_; }
            inline const Volume& volume() const noexcept { return volume_; }
            inline const ColorSpace& colorSpace() const noexcept { return colorSpace_; }
            inline const GeometryReference geometry() const noexcept { return geometry_; }
            inline const std::filesystem::path path() const noexcept { return path_; }
            inline const std::vector<Source>& sources() const noexcept { return sources_; }
            inline unsigned int version() const { return version_; }
            inline unsigned int version(const unsigned int version) { return version_ = version; }

            static unsigned int load_version(const std::filesystem::path &path);
            static unsigned int write_version(const std::filesystem::path &path, unsigned int version);
            static unsigned int increment_version(const std::filesystem::path &path);

        private:
            std::vector<Source> load_sources();

            const Catalog &catalog_;
            const std::string name_;
            const std::filesystem::path path_;
            const Volume volume_;
            const ColorSpace colorSpace_;
            const GeometryReference geometry_;
            unsigned int version_;
            std::vector<Source> sources_;
        };

        class ExternalEntry {
        public:
            explicit ExternalEntry(const std::filesystem::path &filename)
                    : ExternalEntry(filename, {}, {})
            { }

            explicit ExternalEntry(std::filesystem::path filename, const std::optional<Volume> &volume,
                                                                   const std::optional<GeometryReference> &geometry)
                    : filename_(std::move(filename)),
                      sources_(load_sources(volume, geometry))
            { CHECK(std::filesystem::exists(filename_)); }

            inline const std::filesystem::path &filename() const noexcept { return filename_; }
            inline const std::vector<Source>& sources() const noexcept { return sources_; }

        private:
            std::vector<Source> load_sources(const std::optional<Volume>&, const std::optional<GeometryReference>&);

            const std::filesystem::path filename_;
            const std::vector<Source> sources_;
        };
    } //namespace catalog
} //namespace lightdb

#endif //LIGHTDB_CATALOG_H
