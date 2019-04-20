#include "Transaction.h"
#include "Gpac.h"

namespace lightdb::transactions {

std::recursive_mutex Transaction::global{};

std::filesystem::path OutputStream::destination_filename(const unsigned int version) const {
    auto filename = filename_.filename().string();
    // ?-i-stream.mp4#tid -> ?-i-stream.mp4 || foo.mp4#tid -> foo.mp4
    std::string base_filename{filename.begin(),
                              filename.end() - std::to_string(transaction_.id()).size() - 1};

    if(entry().has_value()) {
        // ?-i-stream.mp4 -> v-i-stream.mp4
        return filename_.parent_path() / (base_filename.replace(0, 1, std::to_string(version)));
    } else
        // foo.mp4 -> foo.mp4
        return filename_.parent_path() / base_filename;
}

void SingleNodeVolatileTransaction::commit() {
    complete_ = true;

    std::lock_guard lock{global};

    std::unordered_map<std::string, unsigned int> versions;

    // Get distinct set of outputs
    for(auto &output: outputs())
        if(output.entry().has_value())
            versions[output.entry().value().path().string()] = output.entry().value().version();

    // Increment each distinct output's version once
    for(auto &[path, version]: versions)
        versions[path] = catalog::Entry::increment_version(path);

    // Commit files to new version
    for(auto &output: outputs()) {
        output.stream().close();

        auto committed_filename = output.destination_filename(
                output.entry().has_value()
                    ? versions[output.entry().value().path()]
                    : 0u);

        if(video::gpac::can_mux(output.filename()))
            video::gpac::mux_media(output.filename(), committed_filename, output.codec());
        else
            std::filesystem::rename(output.filename(), committed_filename);
    }

    // Write metadata now that streams have committed
    for(auto &[path, version]: versions)
        write_metadata(path, versions[path]);
}

void SingleNodeVolatileTransaction::abort() {
    complete_ = true;
    for(const auto &output: outputs())
    std::filesystem::remove(output.filename());
}

void SingleNodeVolatileTransaction::write_metadata(const std::filesystem::path& path, const unsigned int version) {
    std::vector<OutputStream> outputs_in_this_path;
    auto filename = catalog::Files::metadata_filename(path, version);

    for(auto index = 0u; index < outputs().size(); index++) {
        const auto &output = outputs().at(index);
        if(output.entry().has_value() && output.entry().value().path() == path)
            outputs_in_this_path.emplace_back(OutputStream(output, path, version, index));
    }

    video::gpac::write_metadata(filename, outputs_in_this_path);
}

} // namespace lightdb::transactions

