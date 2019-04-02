#ifndef LIGHTDB_STOREOPERATORS_H
#define LIGHTDB_STOREOPERATORS_H

#include "PhysicalOperators.h"
#include "Transaction.h"

namespace lightdb::physical {

class Store: public PhysicalOperator {
public:
    explicit Store(const LightFieldReference &logical,
                   PhysicalOperatorReference &parent)
            : Store(logical,
                    std::vector<PhysicalOperatorReference>{parent})
    { }

    explicit Store(const LightFieldReference &logical,
                   const std::vector<PhysicalOperatorReference> &parents)
            : Store(logical,
                    logical->downcast<logical::StoredLightField>(),
                    parents)
    { }

    const Codec &codec() const { return codec_; }

private:
    explicit Store(const LightFieldReference &logical,
                   const logical::StoredLightField &store,
                   const std::vector<PhysicalOperatorReference> &parents)
            : PhysicalOperator(logical, parents, DeviceType::CPU, runtime::make<Runtime>(*this, store)),
              codec_(store.codec()) {
        CHECK_GT(parents.size(), 0);
    }

    class Runtime: public runtime::Runtime<Store> {
    public:
        Runtime(Store &physical, const logical::StoredLightField &logical)
                : runtime::Runtime<Store>(physical),
                  outputs_{functional::transform<std::reference_wrapper<transactions::OutputStream>>(
                          physical.parents().begin(), physical.parents().end(),
                          [this, &logical](auto &parent) {
                              return std::reference_wrapper(this->physical().context()->transaction().write(logical)); }) }
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(!all_parent_eos()) {
                for(auto i = 0u; i < iterators().size(); i++) {
                    auto input = iterators().at(i)++;
                    auto &output =  outputs_.at(i).get();
                    auto &data = input.downcast<SerializableData>();

                    std::copy(data.value().begin(),data.value().end(), std::ostreambuf_iterator<char>(output.stream()));
                }

                return EmptyData(DeviceType::CPU);
            } else
                return std::nullopt;
        }

    private:
        std::vector<std::reference_wrapper<transactions::OutputStream>> outputs_;
    };

    const Codec codec_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_STOREOPERATORS_H
