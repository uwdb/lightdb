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
        class Stream {
        public:
            Stream(std::filesystem::path path, Codec codec, Configuration configuration)
                    : path_(std::move(path)),
                      codec_(std::move(codec)),
                      configuration_(configuration)
            { }

            const std::filesystem::path& path() const { return path_; }
            const Codec& codec() const { return codec_; }
            const Configuration& configuration() const { return configuration_; }
            //TODO
            GeometryReference geometry() const { return GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples{0u, 0u}); }

        private:
            const std::filesystem::path path_;
            const Codec codec_;
            const Configuration configuration_;
        };

        class OutputStream: public Stream {
        public:
            OutputStream(const std::filesystem::path &path, const Codec &codec, const Configuration &configuration)
                    : Stream(path, codec, configuration),
                      stream_(path)
            { }

            std::ofstream& stream() { return stream_; }

        private:
            std::ofstream stream_;
        };

        class Catalog {
        public:
            Catalog(const Catalog &catalog) = default;
            explicit Catalog(std::filesystem::path path)
                    : path_(std::move(asserts::CHECK_NONEMPTY(path)))
            { }

            //TODO these overloads are superfluous, but cause issues with syntax highlighting
            explicit Catalog(const char *path)
                : Catalog(std::filesystem::path(path))
            { }
            explicit Catalog(const std::string &path)
                : Catalog(std::filesystem::path(path))
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

            LightFieldReference get(const std::string &name) const;
            bool exists(const std::string &name) const;
            OutputStream create(const std::string& name, const Codec &codec, const Configuration &configuration) const;

            class Metadata {
                friend class Catalog;

                Metadata(const Catalog &catalog, std::string name, std::filesystem::path path)
                        : catalog_(catalog), name_(std::move(name)), path_(std::move(path)),
                        //TODO grab boxes and load volume/geometry, detect colorspace
                          volume_({{0, 0}, {0, 0}, {0, 0}, {0, 10}, ThetaRange::limits(), PhiRange::limits()}),
                          colorSpace_(YUVColorSpace::instance()),
                        //TODO move geometry to streams
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
                const std::filesystem::path path_;
                const Volume volume_;
                const ColorSpace colorSpace_;
                const GeometryReference geometry_;
                const std::vector<Stream> streams_;
            };

        private:
            Catalog() : path_("") { }

            const std::filesystem::path path_;

            static constexpr const char *metadataFilename_ = "metadata.mp4";
            static std::optional<Catalog> instance_;
            static std::vector<Stream> get_streams(const std::filesystem::path&);
            inline Catalog::Metadata metadata(const std::string &name) const;
        };
    } //namespace catalog
} //namespace lightdb

#endif //LIGHTDB_CATALOG_H
