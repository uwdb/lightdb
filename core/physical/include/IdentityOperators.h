#ifndef LIGHTDB_IDENTITYOPERATORS_H
#define LIGHTDB_IDENTITYOPERATORS_H

#include "MaterializedLightField.h"
#include "PhysicalOperators.h"

namespace lightdb::physical {
    class CPUIdentity: public PhysicalLightField {
    public:
        CPUIdentity(const LightFieldReference &logical)
                : PhysicalLightField(logical, {}, physical::DeviceType::CPU)
        { }

        CPUIdentity(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent)
                : PhysicalLightField(logical, {parent}, physical::DeviceType::CPU) { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterators().front() != iterators().front().eos()) {
                return iterators().front()++;
            } else
                return {};
        }
    };

    class GPUIdentity: public GPUUnaryOperator<GPUDecodedFrameData> {
    public:
        GPUIdentity(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent)
                : GPUUnaryOperator(logical, parent)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterator() != iterator().eos()) {
                return iterator()++;
            } else
                return {};
        }
    };

}; // namespace lightdb::physical

#endif //LIGHTDB_IDENTITYOPERATORS_H
