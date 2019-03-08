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
            auto input = iterators()[0]++;
            auto data = input.downcast<GPUDecodedFrameData>();
            auto &configuration = parents().front().downcast<GPUOperator>().configuration();
            CPUDecodedFrameData output{configuration};

            for(auto &frame: data.frames())
                output.frames().emplace_back(LocalFrame{*frame->cuda(), configuration});

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
            : GPUOperator(logical, {parent}, gpu, [this]() {
                  CHECK_GT(height_, 0);
                  CHECK_GT(width_, 0);
                  return Configuration{width_, height_, 0, 0, 0, {30, 1}, {}}; }),
              height_(0u),
              width_(0u) {
        LOG(WARNING) << "CPU->GPU transfer operator has hardcoded framerate of 30";
    }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(iterators()[0] != iterators()[0].eos()) {
            GPUDecodedFrameData output{configuration()};
            auto input = iterators()[0]++;
            auto data = input.downcast<CPUDecodedFrameData>();

            for(LocalFrameReference &frame: data.frames())
                output.frames().emplace_back(std::make_shared<CudaFrame>(
                        *InitializeConfiguration(frame)));

            return {output};
        } else
            return {};
    }

private:
    unsigned int height_;
    unsigned int width_;

    inline LocalFrameReference InitializeConfiguration(LocalFrameReference &frame) {
        //TODO parent op should be explicitly exposing a configuration, this is a hack
        if(height_ == 0) {
            height_ = frame->height();
            width_ = frame->width();
        }
        return frame;
    }
};

} // lightdb::physical

#endif //LIGHTDB_TRANSFEROPERATORS_H
