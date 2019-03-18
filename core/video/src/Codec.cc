#include "Codec.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace lightdb {
    const Codec& Codec::raw() {
        static const Codec value("RAW", "IRAW", {}, {}, {});
        return value;
    }

    const Codec& Codec::h264() {
        static const Codec value("H264", "H264", NV_ENC_H264, cudaVideoCodec_H264, AV_CODEC_ID_H264);
        return value;
    }

    const Codec& Codec::hevc() {
        static const Codec value{"HEVC", "HEVC", NV_ENC_HEVC, cudaVideoCodec_HEVC, AV_CODEC_ID_HEVC};
        return value;
    }

    const Codec& Codec::boxes() {
        static const Codec value("BoundingBoxes", "BOXS", {}, {}, {});
        return value;
    }
} // namespace lightdb