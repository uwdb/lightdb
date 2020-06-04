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
        const std::optional<NV_ENC_BUFFER_FORMAT>& nvenc_format() const { return nvenc_format_; }

        static const std::vector<Format>& all() {
            static const std::vector<Format> formats{nv12(), nv21(), iyuv(), yv12()};
            return formats;
        }

        static const Format& nv12();
        static const Format& nv21();
        static const Format& iyuv();
        static const Format& yv12();
        static const Format& unknown();

        static const std::optional<Format> get_by_nvenc(const NV_ENC_BUFFER_FORMAT &id) {
            auto format = std::find_if(Format::all().begin(), Format::all().end(),
                                       [&id](const auto &f) { return f.nvenc_format() == id; });
            return format != Format::all().end()
                   ? std::optional<Format>{*format} : std::nullopt;
        }

    private:
        Format(const rational &allocation_height_ratio, const rational &allocation_width_ratio,
               const std::optional<AVPixelFormat> ffmpeg_format,
               const std::optional<NV_ENC_BUFFER_FORMAT> nvenc_format)
                : allocation_height_ratio_(allocation_height_ratio),
                  allocation_width_ratio_(allocation_width_ratio),
                  ffmpeg_format_(ffmpeg_format),
                  nvenc_format_(nvenc_format)
        { }

        const rational allocation_height_ratio_, allocation_width_ratio_;
        std::optional<AVPixelFormat> ffmpeg_format_;
        std::optional<NV_ENC_BUFFER_FORMAT> nvenc_format_;
    };
}

#endif //LIGHTDB_FORMAT_H
