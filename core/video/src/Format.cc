#include "Format.h"

extern "C" {
#include <libavutil/pixfmt.h>
}

namespace lightdb::video {
    const Format &Format::nv12() {
        static const Format nv12{rational{3, 2}, rational{1}, AV_PIX_FMT_NV12, NV_ENC_BUFFER_FORMAT_NV12};
        return nv12;
    }

    const Format &Format::nv21() {
        static const Format nv21{rational{3, 2}, rational{1}, AV_PIX_FMT_NV21, {}};
        return nv21;
    }

    const Format &Format::iyuv() {
        static const Format iyuv{rational{3, 2}, rational{1}, {}, NV_ENC_BUFFER_FORMAT_IYUV};
        return iyuv;
    }

    const Format &Format::yv12() {
        static const Format yv12{rational{2}, rational{1}, AV_PIX_FMT_YUV420P, NV_ENC_BUFFER_FORMAT_YV12};
        return yv12;
    }

    const Format &Format::unknown() {
        static const Format unknown{rational{0}, rational{0}, {}, {}};
        return unknown;
    }
}