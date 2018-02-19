#ifndef LIGHTDB_CATALOG_H
#define LIGHTDB_CATALOG_H

#include "Geometry.h"
#include "Color.h"
#include <experimental/filesystem>
#include <utility>

namespace lightdb {
    class LightFieldReference;

    namespace catalog {
        class Catalog {
        public:
            Catalog(const Catalog &catalog) = default;
            explicit Catalog(std::experimental::filesystem::path path)
                    : path_(std::move(asserts::CHECK_NONEMPTY(path)))
            { }

            //TODO these overloads are superfluous, but cause issues with syntax highlighting
            explicit Catalog(const char *path)
                : Catalog(std::experimental::filesystem::path(path))
            { }
            explicit Catalog(const std::string &path)
                : Catalog(std::experimental::filesystem::path(path))
            { }

            static const Catalog &instance()
            {
                if(instance_.has_value())
                    return instance_.value();
                else
                    throw CatalogError("No ambient catalog specified", "instance");
            }
            static const Catalog &instance(Catalog catalog) { return instance_.emplace(catalog); }

            LightFieldReference get(const std::string &name) const;

            class Metadata {
                friend class Catalog;

                Metadata(const Catalog &catalog, std::experimental::filesystem::path path)
                        : catalog_(catalog), path_(std::move(path)),
                        //TODO grab boxes and load volume/geometry, detect colorspace
                          volume_({{0, 0}, {0, 0}, {0, 0}, {0, 10}, ThetaRange::limits(), PhiRange::limits()}),
                          colorSpace_(YUVColorSpace::Instance),
                          geometry_(EquirectangularGeometry::instance())
                { }

            public:
                const Volume& volume() const { return volume_; }
                const ColorSpace& colorSpace() const { return colorSpace_; }
                const Geometry& geometry() const { return geometry_; }

            private:
                const Catalog &catalog_;
                const std::experimental::filesystem::path path_;
                const Volume volume_;
                const ColorSpace colorSpace_;
                const Geometry &geometry_;
            };

        private:
            Catalog() : path_("") { }

            const std::experimental::filesystem::path path_;
            static constexpr const char *metadataFilename_ = "metadata.mp4";
            static std::optional<Catalog> instance_;
        };
    } //namespace catalog
} //namespace lightdb

#endif //LIGHTDB_CATALOG_H
