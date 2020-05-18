#ifndef LIGHTDB_TRANSFEROPERATORS_H
#define LIGHTDB_TRANSFEROPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class GPUtoCPUTransfer: public PhysicalOperator {
public:
    GPUtoCPUTransfer(const LightFieldReference &logical,
                     PhysicalOperatorReference &parent)
            : PhysicalOperator(logical, {parent}, physical::DeviceType::CPU, runtime::make<Runtime>(*this))
    { }

private:
    class Runtime: public runtime::Runtime<> {
    public:
        explicit Runtime(PhysicalOperator &physical)
            : runtime::Runtime<>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterators()[0] != iterators()[0].eos()) {
                auto input = iterators()[0]++;
                auto data = input.downcast<GPUDecodedFrameData>();
                CPUDecodedFrameData output{data.configuration(), data.geometry()};

                for(auto &frame: data.frames())
                    output.frames().emplace_back(LocalFrame{*frame->cuda(), data.configuration()});

                return {output};
            } else
                return {};
        }
    };
};

class CPUtoGPUTransfer: public PhysicalOperator, public GPUOperator {
public:
    CPUtoGPUTransfer(const LightFieldReference &logical,
                PhysicalOperatorReference &parent,
                const execution::GPU &gpu)
            : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)), //, gpu),
              GPUOperator(gpu)
    { }

private:
    class Runtime: public runtime::GPURuntime<CPUtoGPUTransfer> {
    public:
        explicit Runtime(CPUtoGPUTransfer &physical)
            : runtime::GPURuntime<CPUtoGPUTransfer>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterators().front() != iterators().front().eos()) {
                auto input = iterators().front()++;
                auto data = input.downcast<CPUDecodedFrameData>();
                GPUDecodedFrameData output{data.configuration(), data.geometry()};

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
