#ifndef LIGHTDB_CATALOG_H
#define LIGHTDB_CATALOG_H

#include "Codec.h"
#include "Geometry.h"
#include "Color.h"
#include "Configuration.h"
#include <experimental/filesystem>
#include <utility>

namespace lightdb {
    class LightField;
    namespace logical { class Algebra; }
    using LightFieldReference = shared_reference<LightField, logical::Algebra>;

    namespace catalog {
        class Stream {
        public:
            Stream(std::experimental::filesystem::path path, Codec codec, Configuration configuration)
                    : path_(std::move(path)), codec_(std::move(codec)), configuration_(std::move(configuration))
            { }

            const std::experimental::filesystem::path& path() const { return path_; }
            const Codec& codec() const { return codec_; }
            const Configuration& configuration() const { return configuration_; }

        private:
            const std::experimental::filesystem::path path_;
            const Codec codec_;
            const Configuration configuration_;
        };

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

                Metadata(const Catalog &catalog, std::string name, std::experimental::filesystem::path path)
                        : catalog_(catalog), name_(std::move(name)), path_(std::move(path)),
                        //TODO grab boxes and load volume/geometry, detect colorspace
                          codec_(Codec::h264()),
                          volume_({{0, 0}, {0, 0}, {0, 0}, {0, 10}, ThetaRange::limits(), PhiRange::limits()}),
                          colorSpace_(YUVColorSpace::instance()),
                          geometry_(EquirectangularGeometry(EquirectangularGeometry::Samples())),
                          streams_(get_streams(path_))
                { }

            public:
                const Volume& volume() const noexcept { return volume_; }
                const ColorSpace& colorSpace() const noexcept { return colorSpace_; }
                const GeometryReference geometry() const noexcept { return geometry_; }
                const std::string path() const noexcept { return path_; }
                const std::vector<Stream>& streams() const noexcept { return streams_; }

            private:
                const Catalog &catalog_;
                const std::string name_;
                const std::experimental::filesystem::path path_;
                const Codec codec_;
                const Volume volume_;
                const ColorSpace colorSpace_;
                const GeometryReference geometry_;
                std::vector<Stream> streams_;
            };

        private:
            Catalog() : path_("") { }

            const std::experimental::filesystem::path path_;

            static constexpr const char *metadataFilename_ = "metadata.mp4";
            static std::optional<Catalog> instance_;
            static std::vector<Stream> get_streams(const std::experimental::filesystem::path&);
        };
    } //namespace catalog
} //namespace lightdb

#endif //LIGHTDB_CATALOG_H
