#include "Metadata.pb.h"
#include "Catalog.h"
#include "Codec.h"
#include "Gpac.h"
#include "Transaction.h"
#include "serialization.h"
#include "errors.h"
#include "gpac/isomedia.h"
#include "gpac/media_tools.h"
#include "linux/videodev2.h"

namespace lightdb::video::gpac {
    static auto constexpr METADATA_VERSION = 1u;
    static auto constexpr MP4_EXTENSION = "mp4";
    static auto constexpr TLFD_FOURCC = v4l2_fourcc('d', 'f', 'l', 't');
    static auto _ = []() {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        // Omit mux progress bars
        gf_set_progress_callback(nullptr, [](auto data, auto title, auto done, auto total) {});
        // Disable "unknown box" warnings for TLFD box
        gf_log_set_tool_level(GF_LOG_CONTAINER, GF_LOG_ERROR);
        return true;
    }();

    Codec get_codec(GF_ISOFile *file, unsigned int track, unsigned int stream) {
        if(gf_isom_get_avc_svc_type(file, track, stream) != GF_ISOM_AVCTYPE_NONE)
            return Codec::h264();
        else if(gf_isom_get_hevc_lhvc_type(file, track, stream) != GF_ISOM_HEVCTYPE_NONE)
            return Codec::hevc();
        else
            throw GpacRuntimeError("Unsupported GPAC codec", GF_IO_ERR);
    }

    // This is a temporarly function until GPAC post-0.7.1, when we can use gf_isom_get_bitrate
    unsigned int get_average_bitrate(GF_ISOFile *file, const unsigned int track, const unsigned int stream) {
        std::shared_ptr<GF_DecoderConfig> decoder_configuration(gf_isom_get_decoder_config(file, track, stream));
        if(decoder_configuration == nullptr)
            throw GpacRuntimeError("Could not get decoder configuration from stream", GF_IO_ERR);
        else
            return decoder_configuration->avgBitrate;
    }

    serialization::Metadata load_stream_metadata(GF_ISOFile *file) {
        lightdb::serialization::Metadata metadata;
        unsigned int box_type, size;
        char *raw_data = nullptr;
        GF_Err result;

        if(gf_isom_get_udta_count(file, 0) != 1)
            throw GpacRuntimeError("Could not find metadata box in container", GF_IO_ERR);
        else if((result = gf_isom_get_udta_type(file, 0, 1, &box_type, nullptr)) != GF_OK)
            throw GpacRuntimeError("Could not retrieve user data box type", result);
        else if((result = gf_isom_get_user_data(file, 0, box_type, nullptr, 1, &raw_data, &size)) != GF_OK)
            throw GpacRuntimeError("Could not retrieve user data", result);

        std::shared_ptr<char> data(raw_data);

        if(!metadata.ParseFromArray(data.get(), size))
            throw GpacRuntimeError("Could not parse user data", GF_IO_ERR);
        else
            return metadata;
    }

    std::vector<catalog::Source> load_metadata(const std::filesystem::path &filename) {
        std::vector<catalog::Source> results;
        GF_ISOFile *file;
        unsigned int tracks;
        const char *url, *urn;
        GF_Err result;

        if((file = gf_isom_open(filename.c_str(), GF_ISOM_OPEN_READ_DUMP, nullptr)) == nullptr)
            throw GpacRuntimeError("Error opening file", GF_IO_ERR);
        else if((tracks = gf_isom_get_track_count(file)) == static_cast<unsigned int>(-1))
            throw GpacRuntimeError("Error opening file", GF_IO_ERR);

        auto metadata = load_stream_metadata(file);

        CHECK_EQ(tracks, metadata.entries_size());

        for(auto track = 1u; track < tracks + 1; track++) {
            const auto streams = gf_isom_get_sample_description_count(file, track);
            const auto &entry = metadata.entries(track - 1);

            CHECK_EQ(streams, 1);

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

                    results.emplace_back(
                            track - 1,
                            std::filesystem::absolute(url),
                            get_codec(file, track, stream),
                            Configuration{
                                    width,
                                    height,
                                    0u,
                                    0u,
                                    bitrate,
                                    {scale * samples, static_cast<unsigned int>(duration)},
                                    {static_cast<unsigned int>(left), static_cast<unsigned int>(top)}},
                             serialization::as_composite_volume(entry),
                             serialization::as_geometry(entry));
                }
            }
        }

        if((result = gf_isom_close(file)) != GF_OK)
            throw GpacRuntimeError("Error closing file", result);

        return results;
    }

    void write_metadata(const std::filesystem::path &metadata_filename,
                        const std::vector<std::filesystem::path> &stream_filenames,
                        const serialization::Metadata &metadata) {
        GF_MediaImporter import{};
        GF_Err result;

        import.flags = GF_IMPORT_USE_DATAREF;
        if((import.dest = gf_isom_open(metadata_filename.c_str(), GF_ISOM_OPEN_WRITE, nullptr)) == nullptr)
            throw GpacRuntimeError("Error opening metadata file", GF_IO_ERR);

        for(const auto &stream_filename: stream_filenames) {
            if((import.orig = gf_isom_open(std::filesystem::relative(stream_filename).c_str(), GF_ISOM_OPEN_READ, nullptr)) == nullptr)
                throw GpacRuntimeError("Error opening stream file", GF_IO_ERR);
            else if((result = gf_media_import(&import)) != GF_OK)
                throw GpacRuntimeError("Error importing reference track", result);
            else if((result = gf_isom_close(import.orig)) != GF_OK)
                throw GpacRuntimeError("Error closing file", result);
        }

        auto metadata_string = metadata.SerializeAsString();

        if((result = gf_isom_add_user_data(import.dest, 0, TLFD_FOURCC, nullptr, metadata_string.data(),
                                           static_cast<unsigned int>(metadata_string.length()))) != GF_OK)
            throw GpacRuntimeError("Error writing metadata box", result);
        else if((result = gf_isom_close(import.dest)) != GF_OK)
            throw GpacRuntimeError("Error closing file", result);
    }

    void write_metadata(const std::filesystem::path &metadata_filename,
                        const std::vector<transactions::OutputStream> &outputs) {
        std::vector<std::filesystem::path> filenames;
        lightdb::serialization::Metadata metadata;
        metadata.set_version(METADATA_VERSION);

        for(const auto &output: outputs) {
            auto entry = metadata.add_entries();

            entry->CopyFrom(serialization::as_entry(output));
            filenames.push_back(output.filename());
        }

        write_metadata(metadata_filename, filenames, metadata);
    }

    bool can_mux(const std::filesystem::path &filename) {
        return filename.extension() == MP4_EXTENSION;
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
} // namespace lightdb::video::gpac
