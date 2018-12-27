#include "Catalog.h"
#include "LightField.h"
#include "PhysicalOperators.h"
#include "Model.h"

namespace filesystem = ::std::experimental::filesystem;

namespace lightdb::catalog {
    std::optional<Catalog> Catalog::instance_;

    Catalog::Metadata Catalog::metadata(const std::string &name) const {
        auto path = filesystem::absolute(path_ / name);
        return Catalog::Metadata{*this, name, path};
    }

    LightFieldReference Catalog::get(const std::string &name) const {
        auto path = filesystem::absolute(path_ / name);
        auto metadataFilename = path / metadataFilename_;

        if(!filesystem::exists(metadataFilename))
            throw CatalogError(std::string("Light field ") + name + " does not exist in catalog " + path_.string(), name);
        else
            return LightFieldReference::make<logical::ScannedLightField>(metadata(name));
    }

    OutputStream Catalog::create(const std::string& name, const Codec &codec, const Configuration &configuration) const {
        auto path = filesystem::absolute(path_ / name);
        auto metadataFilename = path / metadataFilename_;
        auto streamFilename = path / "stream0.hevc";

        if(!filesystem::exists(path))
            filesystem::create_directory(path);

        LOG(INFO) << "Assuming TLF version 0";
        LOG(WARNING) << "Not creating catalog metadata";
        return OutputStream(streamFilename, codec, configuration);
    }

    std::vector<Stream> Catalog::get_streams(const filesystem::path &path) {
        LOG(WARNING) << "Using hardcoded first stream for video configuration";

        const size_t index = 0;
        auto configuration = utility::ffmpeg::GetStreamConfiguration(path / metadataFilename_, index, false);

        filesystem::path stream_filename;
        if(filesystem::exists(filesystem::absolute(path / "stream0.hevc")))
            stream_filename = filesystem::absolute(path / "stream0.hevc");
        else if(filesystem::exists(filesystem::absolute(path / "stream0.h264")))
            stream_filename = filesystem::absolute(path / "stream0.h264");
        else if(filesystem::exists(filesystem::absolute(path / "stream0.boxes"))) {
            stream_filename = filesystem::absolute(path / "stream0.boxes");
            //TODO
            return {Stream{stream_filename, Codec::boxes(), static_cast<Configuration&>(configuration)}};
        }

        return {Stream{stream_filename, configuration.codec, static_cast<Configuration&>(configuration)}};
    }

} // namespace lightdb::catalog