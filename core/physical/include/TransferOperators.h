#ifndef LIGHTDB_TRANSFEROPERATORS_H
#define LIGHTDB_TRANSFEROPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class GPUtoCPUTransfer: public PhysicalLightField {
public:
    GPUtoCPUTransfer(const LightFieldReference &logical,
                     PhysicalLightFieldReference &parent)
            : PhysicalLightField(logical, {parent}, physical::DeviceType::CPU, runtime::make<Runtime>(*this))
    { }

private:
    class Runtime: public runtime::Runtime<> {
    public:
        explicit Runtime(PhysicalLightField &physical) : runtime::Runtime<>(physical) { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterators()[0] != iterators()[0].eos()) {
                auto input = iterators()[0]++;
                auto data = input.downcast<GPUDecodedFrameData>();
                CPUDecodedFrameData output{data.configuration()};

                for(auto &frame: data.frames())
                    output.frames().emplace_back(LocalFrame{*frame->cuda(), data.configuration()});

                return {output};
            } else
                return {};
        }
    };
};

class CPUtoGPUTransfer: public GPUOperator {
public:
    CPUtoGPUTransfer(const LightFieldReference &logical,
                PhysicalLightFieldReference &parent,
                const execution::GPU &gpu)
            : GPUOperator(logical, {parent}, runtime::make<Runtime>(*this), gpu)
    { }

private:
    class Runtime: public GPUOperator::Runtime<CPUtoGPUTransfer> {
    public:
        explicit Runtime(CPUtoGPUTransfer &physical)
            : GPUOperator::Runtime<CPUtoGPUTransfer>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterators().front() != iterators().front().eos()) {
                auto input = iterators().front()++;
                auto data = input.downcast<CPUDecodedFrameData>();
                GPUDecodedFrameData output{data.configuration()};

                for(LocalFrameReference &frame: data.frames())
                    output.frames().emplace_back(std::make_shared<CudaFrame>(*frame));

                return {output};
            } else
                return {};
        }
    };
};

} // lightdb::physical

#endif //LIGHTDB_TRANSFEROPERATORS_H
