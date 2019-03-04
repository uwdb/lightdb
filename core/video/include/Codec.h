#ifndef LIGHTDB_CODEC_H
#define LIGHTDB_CODEC_H

#include "EncodeAPI.h"
#include <vector>

namespace lightdb {

    class Codec {
    public:
        static const Codec& raw() {
            static const Codec value("RAW", "IRAW", {}, {});
            return value;
        }

        static const Codec& h264() {
            static const Codec value("H264", "H264", NV_ENC_H264, cudaVideoCodec_H264);
            return value;
        }

        static const Codec& hevc() {
            static const Codec value{"HEVC", "HEVC", NV_ENC_HEVC, cudaVideoCodec_HEVC};
            return value;
        }

        static const Codec& boxes() {
            static const Codec value("BoundingBoxes", "BOXS", {}, {});
            return value;
        }

        static const std::vector<Codec>& all() {
            static const std::vector<Codec> codecs{raw(), h264(), hevc(), boxes()};
            return codecs;
        }

        static const std::optional<Codec> get(const std::string &name) {
            auto codec = std::find_if(Codec::all().begin(), Codec::all().end(),
                                      [&name](const auto &c) { return strncasecmp(name.c_str(), c.name().c_str(), name.length()) == 0 ||
                                                                      strncasecmp(name.c_str(), c.extension().c_str(), name.length()) == 0; });
            return codec != Codec::all().end()
                ? std::optional<Codec>{*codec} : std::nullopt;
        }

        Codec(Codec&&) = default;
        Codec(const Codec&) = default;

        const std::string& name() const { return name_; }
        const std::string& fourcc() const { return fourcc_; }
        const std::string& extension() const { return extension_; }
        const std::optional<EncodeCodec>& nvidiaId() const { return nvidiaId_; }
        const std::optional<cudaVideoCodec>& cudaId() const { return cudaId_; }

        bool operator==(const Codec &other) const {
            return name_ == other.name_ &&
                   fourcc_ == other.fourcc_ &&
                   nvidiaId_ == other.nvidiaId_ &&
                   cudaId_ == other.cudaId_;
        }

        bool operator!=(const Codec &other) const {
            return !(*this == other);
        }

    private:
        explicit Codec(std::string name, std::string fourcc,
                std::optional<EncodeCodec> nvidiaId={}, std::optional<cudaVideoCodec> cudaId={}) noexcept
                : name_(std::move(name)), fourcc_(std::move(fourcc)), extension_('.' + name_),
                  nvidiaId_{nvidiaId}, cudaId_{cudaId}
        { CHECK_EQ(fourcc_.size(), 4); }

        const std::string name_;
        const std::string fourcc_;
        const std::string extension_;
        const std::optional<EncodeCodec> nvidiaId_;
        const std::optional<cudaVideoCodec> cudaId_;
    };

}

#endif //LIGHTDB_CODEC_H
