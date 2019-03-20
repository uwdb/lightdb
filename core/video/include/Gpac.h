#ifndef LIGHTDB_GPAC_H
#define LIGHTDB_GPAC_H

#include <vector>
#include <filesystem>

namespace lightdb::video::gpac {
    std::vector<catalog::Stream> GetStreams(const std::filesystem::path&);
} // namespace lightdb::video::gpac

#endif //LIGHTDB_GPAC_H
