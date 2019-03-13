#ifndef LIGHTDB_SINKOPERATORS_H
#define LIGHTDB_SINKOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class Sink: public PhysicalLightField {
public:
    explicit Sink(const LightFieldReference &logical,
                   PhysicalLightFieldReference &parent)
        : PhysicalLightField(logical, {parent}, DeviceType::CPU, runtime::make<Runtime>(*this))
    { }

private:
    class Runtime: public runtime::Runtime<> {
    public:
        explicit Runtime(PhysicalLightField &physical) : runtime::Runtime<>(physical) { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(!all_parent_eos()) {
                std::for_each(iterators().begin(), iterators().end(), [](auto &i) { i++; });
                return EmptyData{physical().device()};
            } else
                return std::nullopt;
        }
    };
};

} // namespace lightdb::physical

#endif //LIGHTDB_SINKOPERATORS_H
