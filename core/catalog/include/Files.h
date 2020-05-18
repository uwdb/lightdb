#ifndef LIGHTDB_FILES_H
#define LIGHTDB_FILES_H

#include "reference.h"
#include <filesystem>

namespace lightdb::transactions {
    class Transaction;
}

namespace lightdb::catalog {

class Entry;

class Files {
public:
    static std::filesystem::path metadata_filename(const std::filesystem::path &path,
                                                   const unsigned int version) {
        return path / (std::to_string(version) + metadata_suffix_); }

    static std::filesystem::path stream_filename(const std::filesystem::path &path, const unsigned int version,
                                                 const unsigned int index) {
        return path / (std::to_string(version) + '-' + std::to_string(index) + stream_suffix_); }

    static std::filesystem::path staging_filename(const transactions::Transaction&, const catalog::Entry&);
    static std::filesystem::path staging_filename(const transactions::Transaction&, const std::filesystem::path&);

    static std::filesystem::path version_filename(const std::filesystem::path &path) {
        return path / version_filename_;
    }

private:
    Files() = default;

    static constexpr auto version_filename_ = "version";
    static constexpr auto metadata_suffix_ = "-metadata.mp4";
    static constexpr auto stream_suffix_ = "-stream.mp4";
};

} // namespace lightdb::catalog
#endif //LIGHTDB_FILES_H
