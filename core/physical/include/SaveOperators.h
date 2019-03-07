#ifndef LIGHTDB_SAVEOPERATORS_H
#define LIGHTDB_SAVEOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class SaveToFile: public PhysicalLightField {
public:
    explicit SaveToFile(const LightFieldReference &logical,
                        PhysicalLightFieldReference &parent)
            : PhysicalLightField(logical, {parent}, DeviceType::CPU),
              out_([this]() { return std::ofstream(this->logical().downcast<logical::SavedLightField>().filename()); }) {
        CHECK_EQ(parents().size(), 1);
    }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(!all_parent_eos()) {
            auto data = iterators().at(0)++;
            auto &serialized = data.downcast<physical::SerializedData>();

            std::copy(serialized.value().begin(), serialized.value().end(),
                      std::ostreambuf_iterator<char>(out_));
            return data;
        } else {
            out_.value().close();
            return std::nullopt;
        }
    }

private:
    lazy<std::ofstream> out_;
};

class CopyFile: public PhysicalLightField {
public:
    explicit CopyFile(const LightFieldReference &logical,
                      const std::filesystem::path &destination)
            : CopyFile(logical, destination, {}) { }

    explicit CopyFile(const LightFieldReference &logical,
                      const std::filesystem::path &destination,
                      PhysicalLightFieldReference &parent)
            : CopyFile(logical, destination, std::vector<PhysicalLightFieldReference>{parent}) { }

    explicit CopyFile(const LightFieldReference &logical,
                      std::filesystem::path destination,
                      const std::vector<PhysicalLightFieldReference> &parents)
            : PhysicalLightField(logical, parents, DeviceType::CPU),
              destination_{std::move(destination)} { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(!copied_) {
            copied_ = true;
            std::filesystem::copy(logical().downcast<logical::ExternalLightField>().stream().path(),
                                  destination_,
                                  std::filesystem::copy_options::overwrite_existing);
            return EmptyData{DeviceType::CPU};
        } else
            return std::nullopt;
    }

private:
    const std::filesystem::path destination_;
    bool copied_ = false;
};

} // namespace lightdb::physical

#endif //LIGHTDB_SAVEOPERATORS_H
