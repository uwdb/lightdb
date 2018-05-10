#ifndef LIGHTDB_CODEC_H
#define LIGHTDB_CODEC_H

#include "EncodeAPI.h"

namespace lightdb {

    class Codec {
    public:
        static const Codec& h264() {
            static const Codec value(std::string("H264"), NV_ENC_H264, cudaVideoCodec_H264);
            return value;
        }

        static const Codec& hevc() {
            static const Codec value{"HEVC", NV_ENC_HEVC, cudaVideoCodec_HEVC};
            return value;
        }

        const std::string& name() const { return name_; }
        const std::optional<EncodeCodec>& nvidiaId() const { return nvidiaId_; }
        const std::optional<cudaVideoCodec>& cudaId() const { return cudaId_; }

    private:
        explicit Codec(std::string name, EncodeCodec nvidiaId={}, cudaVideoCodec cudaId={}) noexcept
                : name_(std::move(name)), nvidiaId_{nvidiaId}, cudaId_{cudaId}
        { }

        const std::string name_;
        const std::optional<EncodeCodec> nvidiaId_;
        const std::optional<cudaVideoCodec> cudaId_;
    };

}

#endif //LIGHTDB_CODEC_H
