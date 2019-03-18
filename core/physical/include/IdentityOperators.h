#ifndef LIGHTDB_IDENTITYOPERATORS_H
#define LIGHTDB_IDENTITYOPERATORS_H

#include "MaterializedLightField.h"
#include "PhysicalOperators.h"

namespace lightdb::physical {
    class CPUIdentity: public PhysicalOperator {
    public:
        explicit CPUIdentity(const LightFieldReference &logical)
                : PhysicalOperator(logical, {}, physical::DeviceType::CPU, runtime::make<Runtime>(*this))
        { }

        CPUIdentity(const LightFieldReference &logical, PhysicalOperatorReference &parent)
                : PhysicalOperator(logical, {parent}, physical::DeviceType::CPU, runtime::make<Runtime>(*this))
        { }

    private:
        class Runtime: public runtime::Runtime<> {
        public:
            explicit Runtime(PhysicalOperator &physical)
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

    class GPUIdentity: public PhysicalOperator, public GPUOperator {
    public:
        GPUIdentity(const LightFieldReference &logical,
                    PhysicalOperatorReference &parent)
                : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)),
                  GPUOperator(parent)
        { }

    private:
        class Runtime: public runtime::UnaryRuntime<GPUIdentity, GPUDecodedFrameData> {
        public:
            explicit Runtime(GPUIdentity &physical)
                : runtime::UnaryRuntime<GPUIdentity, GPUDecodedFrameData>(physical)
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
