#ifndef LIGHTDB_TRANSFEROPERATORS_H
#define LIGHTDB_TRANSFEROPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class GPUtoCPUTransfer: public PhysicalLightField {
public:
    GPUtoCPUTransfer(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent)
            : PhysicalLightField(logical, {parent}, physical::DeviceType::CPU)
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(iterators()[0] != iterators()[0].eos()) {
            CPUDecodedFrameData output;
            auto input = iterators()[0]++;
            auto data = input.downcast<GPUDecodedFrameData>();

            for(auto &frame: data.frames())
                output.frames().emplace_back(LocalFrame{*frame->cuda()});

            return {output};
        } else
            return {};
    }
};

class CPUtoGPUTransfer: public GPUOperator {
public:
    CPUtoGPUTransfer(const LightFieldReference &logical,
                PhysicalLightFieldReference &parent,
                const execution::GPU &gpu)
            : GPUOperator(logical, {parent}, gpu, []() { return Configuration{320, 240, 0, 0, 0, {30, 1}}; })
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(iterators()[0] != iterators()[0].eos()) {
            GPUDecodedFrameData output;
            auto input = iterators()[0]++;
            auto data = input.downcast<CPUDecodedFrameData>();

            for(LocalFrameReference &frame: data.frames())
                output.frames().emplace_back(std::make_shared<CudaFrame>(*frame));

            return {output};
        } else
            return {};
    }
};

} // lightdb::physical

#endif //LIGHTDB_TRANSFEROPERATORS_H
