#ifndef LIGHTDB_FFMPEG_H
#define LIGHTDB_FFMPEG_H

#include "Configuration.h"
#include "Codec.h"

namespace lightdb::video::ffmpeg {
    std::vector<DecodeConfiguration> GetStreamConfigurations(const std::string &filename, bool probe=false);
    DecodeConfiguration GetStreamConfiguration(const std::string &filename, size_t index, bool probe=false);
}; // namespace lightdb::video::ffmpeg

#endif //LIGHTDB_FFMPEG_H
