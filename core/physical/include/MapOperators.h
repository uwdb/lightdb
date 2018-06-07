#ifndef LIGHTDB_MAPOPERATORS_H
#define LIGHTDB_MAPOPERATORS_H

#include <utility>

#include "PhysicalOperators.h"
#include "EncodeOperators.h"

namespace lightdb::physical {

class GPUMap: public GPUUnaryOperator<GPUDecodedFrameData> {
public:
    GPUMap(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent,
           const FrameTransform& transform)
            : GPUMap(logical, parent, [this]() -> Configuration { return this->parent<GPUOperator>().configuration(); },
                     transform)
    { }

    GPUMap(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent,
           const Configuration &configuration,
           FrameTransform transform)
            : GPUUnaryOperator(logical, parent,
                          parent.downcast<GPUOperator>().gpu(),
                          [configuration]() { return configuration; }),
              transform_(std::move(transform))
    { }

    std::optional<physical::DataReference> read() override {
        if(iterator() != iterator().eos()) {
            GPUDecodedFrameData output;
            auto input = iterator()++;

            for(auto& frame: input.frames()) {
                //auto mapped = GPUFrameReference::make<CudaFrame>(configuration().height, configuration().width, frame->type());
                //auto &cuda = mapped.downcast<CudaFrame>();

                auto result = transform_(lock(), *frame->cuda());

                output.frames().emplace_back(GPUFrameReference{frame}); //result});
            }

            return {output};
        } else
            return {};
    }

private:
    GPUMap(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent,
           const std::function<Configuration()> &configuration,
           FrameTransform transform)
            : GPUUnaryOperator(logical, parent, configuration),
              transform_(std::move(transform))
    { }

    const FrameTransform transform_;
};

} // lightdb::physical

#endif //LIGHTDB_MAPOPERATORS_H
