#ifndef LIGHTDB_GPAC_H
#define LIGHTDB_GPAC_H

#include "Transaction.h"
#include <vector>
#include <filesystem>

namespace lightdb::video::gpac {
    std::vector<catalog::Source> load_metadata(const std::filesystem::path&, bool strict=true,
                                               const std::optional<Volume>& ={},
                                               const std::optional<GeometryReference>& ={});
    void write_metadata(const std::filesystem::path &metadata_filename,
                        const std::vector<transactions::OutputStream>&);
    bool can_mux(const std::filesystem::path&);
    void mux_media(const std::filesystem::path &source, const std::filesystem::path &destination,
                   const std::optional<Codec>& ={}, bool remove_source=true);
    } // namespace lightdb::video::gpac

#endif //LIGHTDB_GPAC_H
