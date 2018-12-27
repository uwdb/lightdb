#ifndef LIGHTDB_FFMPEG_H
#define LIGHTDB_FFMPEG_H

#include "Configuration.h"
#include "Codec.h"

namespace lightdb::utility::ffmpeg {
    DecodeConfiguration GetStreamConfiguration(const std::string &filename, size_t index, bool probe=false);
}; // namespace lightdb::utility::ffmpeg

#endif //LIGHTDB_FFMPEG_H
