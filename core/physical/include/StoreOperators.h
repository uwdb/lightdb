#ifndef LIGHTDB_STOREOPERATORS_H
#define LIGHTDB_STOREOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class Store: public PhysicalOperator {
public:
    explicit Store(const LightFieldReference &logical,
                   PhysicalOperatorReference &parent)
            : Store(logical,
                    logical->downcast<logical::StoredLightField>(),
                    parent)
    { }

    const Codec &codec() const { return store_.codec(); }
    const logical::StoredLightField store() const { return store_; }

private:
    explicit Store(const LightFieldReference &logical,
                   const logical::StoredLightField &store,
                   PhysicalOperatorReference &parent)
            : PhysicalOperator(logical, {parent}, DeviceType::CPU, runtime::make<Runtime>(*this, store)),
              store_(store) {
        CHECK_EQ(parents().size(), 1);
    }

    class Runtime: public runtime::UnaryRuntime<Store, CPUEncodedFrameData> {
    public:
        explicit Runtime(Store &physical, const logical::StoredLightField &logical)
            : runtime::UnaryRuntime<Store, CPUEncodedFrameData>(physical),
              output_(physical.store().catalog().create(
                          logical.name(),
                          logical.codec(),
                          configuration()))
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(!all_parent_eos()) {
                auto data = iterator()++;
                std::copy(data.value().begin(),data.value().end(), std::ostreambuf_iterator<char>(output_.stream()));
                return data;
            } else
                return std::nullopt;
        }

    private:
        catalog::OutputStream output_;
    };

    const logical::StoredLightField &store_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_STOREOPERATORS_H
