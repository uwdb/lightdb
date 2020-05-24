#include "Files.h"
#include "Transaction.h"

namespace lightdb::catalog {
    std::filesystem::path Files::staging_filename(const transactions::Transaction &transaction,
                                                  const catalog::Entry &entry) {
        assert(entry.path().is_absolute());

        auto index = std::count_if(transaction.outputs().begin(), transaction.outputs().end(), [&entry](auto &output) {
            return output.entry().has_value() && output.entry().value().path() == entry.path();
        });

        // ?-i-stream.mp4#tid
        auto filename = std::string("?-") + std::to_string(index) + stream_suffix_ +
                        '#' + std::to_string(transaction.id());
        return entry.path() / filename;
    }

    std::filesystem::path Files::staging_filename(const transactions::Transaction &transaction,
                                                  const std::filesystem::path &path) {
        // foo.mp4#tid
        auto filename = path.filename().string() + '#' + std::to_string(transaction.id());
        return path.parent_path() / filename;
    }
} // namespace lightdb::catalog