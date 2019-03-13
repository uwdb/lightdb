#ifndef LIGHTDB_IDENTITYOPERATORS_H
#define LIGHTDB_IDENTITYOPERATORS_H

#include "MaterializedLightField.h"
#include "PhysicalOperators.h"

namespace lightdb::physical {
    class CPUIdentity: public PhysicalLightField {
    public:
        explicit CPUIdentity(const LightFieldReference &logical)
                : PhysicalLightField(logical, {}, physical::DeviceType::CPU, runtime::make<Runtime>(*this))
        { }

        CPUIdentity(const LightFieldReference &logical, PhysicalLightFieldReference &parent)
                : PhysicalLightField(logical, {parent}, physical::DeviceType::CPU, runtime::make<Runtime>(*this))
        { }

    private:
        class Runtime: public runtime::Runtime<> {
        public:
            explicit Runtime(PhysicalLightField &physical)
                : runtime::Runtime<>(physical)
            { }

            std::optional<physical::MaterializedLightFieldReference> read() override {
                if(iterators().front() != iterators().front().eos()) {
                    return iterators().front()++;
                } else
                    return {};
            }
        };
    };

    class GPUIdentity: public GPUUnaryOperator {
    public:
        GPUIdentity(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent)
                : GPUUnaryOperator(logical, parent, runtime::make<Runtime>(*this))
        { }

    private:
        class Runtime: public GPUUnaryOperator::Runtime<GPUIdentity, GPUDecodedFrameData> {
        public:
            explicit Runtime(GPUIdentity &physical)
                : GPUUnaryOperator::Runtime<GPUIdentity, GPUDecodedFrameData>(physical)
            { }

            std::optional<physical::MaterializedLightFieldReference> read() override {
                if(iterator() != iterator().eos()) {
                    return iterator()++;
                } else
                    return {};
            }
        };
    };
}; // namespace lightdb::physical

#endif //LIGHTDB_IDENTITYOPERATORS_H
