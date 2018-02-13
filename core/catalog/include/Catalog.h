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
            explicit Catalog(std::experimental::filesystem::path path)
                : path_(std::move(path))
            { }

            LightFieldReference get(const std::string &name) const;

            class Metadata {
                friend class Catalog;

                Metadata(const Catalog &catalog, std::experimental::filesystem::path path)
                        : catalog_(catalog), path_(std::move(path)),
                        //TODO grab boxes and load volume/geometry, detect colorspace
                          volume_(Volume::VolumeMax), colorSpace_(YUVColorSpace::Instance),
                          geometry_(EquirectangularGeometry::Instance)
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
            const std::experimental::filesystem::path path_;

        };
    } //namespace catalog
} //namespace lightdb

#endif //LIGHTDB_CATALOG_H
