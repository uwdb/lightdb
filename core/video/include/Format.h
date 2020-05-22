#ifndef LIGHTDB_FORMAT_H
#define LIGHTDB_FORMAT_H

#include <cstddef>
#include "number.h"
#include "errors.h"

namespace lightdb::video {
    using AVPixelFormat = unsigned int;

    class Format {
    public:

        size_t allocation_height(const size_t height) const
            { return static_cast<size_t>(height * allocation_height_ratio_); }
        size_t allocation_width(const size_t width) const
            { return static_cast<size_t>(width * allocation_width_ratio_); }
        const std::optional<AVPixelFormat>& ffmpeg_format() const { return ffmpeg_format_; }

        static const Format& nv12();
        static const Format& nv21();
        static const Format& iyuv();
        static const Format& yv12();
        static const Format& unknown();

        static const Format& get_format(const NV_ENC_BUFFER_FORMAT format) {
            switch(format) {
                case NV_ENC_BUFFER_FORMAT_NV12:
                    return nv12();
                case NV_ENC_BUFFER_FORMAT_IYUV:
                    return iyuv();
                case NV_ENC_BUFFER_FORMAT_YV12:
                    return yv12();
                default:
                    throw InvalidArgumentError("Unsupported encoded buffer format", "input_buffer.buffer_format");
            }
        }

    private:
        Format(const rational &allocation_height_ratio, const rational &allocation_width_ratio,
               const std::optional<AVPixelFormat> ffmpeg_format)
                : allocation_height_ratio_(allocation_height_ratio),
                  allocation_width_ratio_(allocation_width_ratio),
                  ffmpeg_format_(ffmpeg_format)
        { }

        const rational allocation_height_ratio_, allocation_width_ratio_;
        std::optional<AVPixelFormat> ffmpeg_format_;
    };
}

#endif //LIGHTDB_FORMAT_H
