#include "Catalog.h"
#include "Codec.h"
#include "errors.h"
#include "Gpac.h"
#include "isomedia.h"

namespace lightdb::video::gpac {
    Codec GetStreamCodec(GF_ISOFile *file, unsigned int track, unsigned int stream) {
        if(gf_isom_get_avc_svc_type(file, track, stream) != GF_ISOM_AVCTYPE_NONE)
            return Codec::h264();
        else if(gf_isom_get_hevc_lhvc_type(file, track, stream) != GF_ISOM_HEVCTYPE_NONE)
            return Codec::hevc();
        else
            throw GpacRuntimeError("Unsupported GPAC codec", GF_IO_ERR);
    }

    std::vector<catalog::Stream> GetStreams(const std::filesystem::path &filename) {
        std::vector<catalog::Stream> results;
        GF_ISOFile *file;
        unsigned int tracks;
        const char *url, *urn;
        GF_Err result;

        if((file = gf_isom_open(filename.c_str(), GF_ISOM_OPEN_READ_DUMP, nullptr)) == nullptr)
            throw GpacRuntimeError("Error opening file", GF_IO_ERR);
        else if((tracks = gf_isom_get_track_count(file)) == std::numeric_limits<unsigned int>::max())
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
                    unsigned int samples, scale, bitrate;
                    unsigned long duration;
                    int left, top;

                    if((result = gf_isom_get_bitrate(file, track, stream, &bitrate, nullptr, nullptr)) != GF_OK)
                        throw GpacRuntimeError("Unexpected error getting bitrate", result);
                    else if((samples = gf_isom_get_sample_count(file, track)) == 0)
                        throw GpacRuntimeError("Unexpected error getting sample count", GF_NOT_FOUND);
                    else if((duration = gf_isom_get_media_original_duration(file, track)) == 0)
                        throw GpacRuntimeError("Unexpected error getting duration", GF_NOT_FOUND);
                    else if((scale = gf_isom_get_media_timescale(file, track)) == 0)
                        throw GpacRuntimeError("Unexpected error getting scale", GF_NOT_FOUND);
                    else if((result = gf_isom_get_track_layout_info(file, track, &width, &height, &left, &top, nullptr)) != GF_OK)
                        throw GpacRuntimeError("Unexpected error getting track layout", result);

                    CHECK_GE(left, 0);
                    CHECK_GE(top, 0);

                    std::filesystem::path stream_filename(url);

                    results.emplace_back(
                            stream_filename.is_relative()
                            ? filename.parent_path() / stream_filename
                            : stream_filename,
                            GetStreamCodec(file, track, stream),
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
} // namespace lightdb::video::gpac
