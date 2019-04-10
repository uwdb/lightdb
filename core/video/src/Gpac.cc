#include "Catalog.h"
#include "Codec.h"
#include "errors.h"
#include "Gpac.h"
#include "gpac/isomedia.h"
#include "gpac/media_tools.h"

namespace lightdb::video::gpac {
    auto disable_gpac_progress_stderr = []() {
        gf_set_progress_callback(nullptr, [](auto data, auto title, auto done, auto total) {});
        return true;
    }();

    Codec get_codec(GF_ISOFile *file, unsigned int track, unsigned int stream) {
        if(gf_isom_get_avc_svc_type(file, track, stream) != GF_ISOM_AVCTYPE_NONE)
            return Codec::h264();
        else if(gf_isom_get_hevc_shvc_type(file, track, stream) != GF_ISOM_HEVCTYPE_NONE)
            return Codec::hevc();
        else
            throw GpacRuntimeError("Unsupported GPAC codec", GF_IO_ERR);
    }

    // This is a temporarly function until GPAC 0.7, when we can use gf_isom_get_bitrate
    unsigned int get_average_bitrate(GF_ISOFile *file, const unsigned int track, const unsigned int stream) {
        std::shared_ptr<GF_DecoderConfig> decoder_configuration(gf_isom_get_decoder_config(file, track, stream));
        if(decoder_configuration == nullptr)
            throw GpacRuntimeError("Could not get decoder configuration from stream", GF_IO_ERR);
        else
            return decoder_configuration->avgBitrate;
    }

    std::vector<catalog::Source> get_streams(const std::filesystem::path &filename) {
        std::vector<catalog::Source> results;
        GF_ISOFile *file;
        unsigned int tracks;
        const char *url, *urn;
        GF_Err result;

        if((file = gf_isom_open(filename.c_str(), GF_ISOM_OPEN_READ_DUMP, nullptr)) == nullptr)
            throw GpacRuntimeError("Error opening file", GF_IO_ERR);
        else if((tracks = gf_isom_get_track_count(file)) == static_cast<unsigned int>(-1))
            throw GpacRuntimeError("Error opening file", GF_IO_ERR);

        for(auto track = 1u; track < tracks + 1; track++) {
            auto streams = gf_isom_get_sample_description_count(file, track);
            for(auto stream = 1u; stream < streams + 1; stream++) {
                if((result = gf_isom_get_data_reference(file, track, stream, &url, &urn)) != GF_OK)
                    throw GpacRuntimeError(gf_error_to_string(result), result);
                else if(url == nullptr)
                    throw GpacRuntimeError("No data reference associated with stream", GF_NOT_SUPPORTED);
                else if(urn != nullptr)
                    throw GpacRuntimeError("Unexpected urn associated with stream", GF_NOT_SUPPORTED);
                else {
                    unsigned int height, width;
                    unsigned int samples, scale;
                    unsigned long duration;
                    int left, top;

                    auto bitrate = get_average_bitrate(file, track, stream);

                    if((samples = gf_isom_get_sample_count(file, track)) == 0)
                        throw GpacRuntimeError("Unexpected error getting sample count", GF_NOT_FOUND);
                    else if((duration = gf_isom_get_media_duration(file, track)) == 0)
                        throw GpacRuntimeError("Unexpected error getting duration", GF_NOT_FOUND);
                    else if((scale = gf_isom_get_media_timescale(file, track)) == 0)
                        throw GpacRuntimeError("Unexpected error getting scale", GF_NOT_FOUND);
                    else if((result = gf_isom_get_track_layout_info(file, track, &width, &height, &left, &top, nullptr)) != GF_OK)
                        throw GpacRuntimeError("Unexpected error getting track layout", result);

                    CHECK_GE(left, 0);
                    CHECK_GE(top, 0);

                    std::filesystem::path stream_filename(url);

                    results.emplace_back(
                            0u,
                            stream_filename.is_relative()
                            ? filename.parent_path() / stream_filename
                            : stream_filename,
                            get_codec(file, track, stream),
                            Configuration{
                                    width,
                                    height,
                                    0u,
                                    0u,
                                    bitrate,
                                    {samples * static_cast<unsigned int>(duration), scale},
                                    {static_cast<unsigned int>(left), static_cast<unsigned int>(top)}
                            });
                }
            }
        }

        if((result = gf_isom_close(file)) != GF_OK)
            throw GpacRuntimeError("Error closing file", result);

        return results;
    }

    void mux_media(const std::filesystem::path &source,
                   const std::filesystem::path &destination,
                   const std::optional<Codec> &codec,
                   const bool remove_source) {
        GF_MediaImporter import{};
        GF_Err result;
        auto input = source.string();
        auto extension = std::string(codec.has_value() ? codec.value().extension() : source.extension().string());

        import.in_name = input.data();
        import.force_ext = extension.data();

        if((import.dest = gf_isom_open(destination.c_str(), GF_ISOM_OPEN_WRITE, nullptr)) == nullptr)
            throw GpacRuntimeError("Error opening destination file", GF_IO_ERR);
        else if((result = gf_media_import(&import)) != GF_OK)
            throw GpacRuntimeError("Error importing track", result);
        else if((result = gf_isom_close(import.dest)) != GF_OK)
            throw GpacRuntimeError("Error closing file", result);
        else if(remove_source && !std::filesystem::remove(source))
            throw InvalidArgumentError("Error deleting source file", "source");
    }

    void write_metadata(const std::filesystem::path &metadata_filename,
                        const std::vector<std::filesystem::path> &stream_filenames) {
        GF_MediaImporter import{};
        GF_Err result;

        import.flags = GF_IMPORT_USE_DATAREF;

        if((import.dest = gf_isom_open(metadata_filename.c_str(), GF_ISOM_OPEN_WRITE, nullptr)) == nullptr)
            throw GpacRuntimeError("Error opening metadata file", GF_IO_ERR);

        for(const auto &stream_filename: stream_filenames) {
            if((import.orig = gf_isom_open(stream_filename.c_str(), GF_ISOM_OPEN_READ, nullptr)) == nullptr)
                throw GpacRuntimeError("Error opening stream file", GF_IO_ERR);
            else if((result = gf_media_import(&import)) != GF_OK)
                throw GpacRuntimeError("Error importing reference track", result);
            else if((result = gf_isom_close(import.orig)) != GF_OK)
                throw GpacRuntimeError("Error closing file", result);
        }

        if((result = gf_isom_close(import.dest)) != GF_OK)
            throw GpacRuntimeError("Error closing file", result);
    }

} // namespace lightdb::video::gpac
