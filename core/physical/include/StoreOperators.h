#ifndef LIGHTDB_STOREOPERATORS_H
#define LIGHTDB_STOREOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class Store: public PhysicalLightField, public EncodedVideoOperator {
public:
    explicit Store(const LightFieldReference &logical,
                   PhysicalLightFieldReference &parent)
            : Store(logical,
                    logical->downcast<logical::StoredLightField>(),
                    parent)
    { }

    const Codec &codec() const override { return store_.codec(); }

private:
    explicit Store(const LightFieldReference &logical,
                   const logical::StoredLightField &store,
                   PhysicalLightFieldReference &parent)
            : PhysicalLightField(logical, {parent}, DeviceType::CPU, runtime::make<Runtime>(*this, store)),
              store_(store) {
        LOG(INFO) << "Storing to ambient catalog";
        CHECK_EQ(parents().size(), 1);
    }

    class Runtime: public runtime::UnaryRuntime<Store, CPUEncodedFrameData> {
    public:
        explicit Runtime(Store &physical, const logical::StoredLightField &logical)
            : runtime::UnaryRuntime<Store, CPUEncodedFrameData>(physical),
              output_(catalog::Catalog::instance().create(
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
