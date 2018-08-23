#ifndef LIGHTDB_INTERPOLATEOPERATORS_H
#define LIGHTDB_INTERPOLATEOPERATORS_H

#include <utility>

#include "PhysicalOperators.h"
#include "EncodeOperators.h"
#include "Functor.h"

namespace lightdb::physical {

class GPUInterpolate: public GPUUnaryOperator<GPUDecodedFrameData> {
public:
    GPUInterpolate(const LightFieldReference &logical,
                   PhysicalLightFieldReference &parent,
                   const interpolation::InterpolatorReference &interpolator)
            : GPUInterpolate(logical, parent,
                             [this]() -> Configuration { return this->parent<GPUOperator>().configuration(); },
                             interpolator)
    { }

    GPUInterpolate(const LightFieldReference &logical,
                   PhysicalLightFieldReference &parent,
                   const Configuration &configuration,
                   const interpolation::InterpolatorReference &interpolator)
            : GPUInterpolate(logical, parent, [configuration]() { return configuration; },
                             interpolator)
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(iterator() != iterator().eos()) {
            auto input = iterator()++;

            auto i = MaterializedLightFieldReference::make<physical::InterpolatedGPUDecodedFrameData>(input, interpolator());
            return i;
            //return MaterializedLightFieldReference::make<physical::InterpolatedGPUDecodedFrameData>(input, interpolator());
        } else
            return {};
    }

    interpolation::InterpolatorReference interpolator() const { return interpolator_; }

private:
    GPUInterpolate(const LightFieldReference &logical,
                   PhysicalLightFieldReference &parent,
                   const std::function<Configuration()> &configuration,
                   const interpolation::InterpolatorReference &interpolator)
            : GPUUnaryOperator(logical, parent, configuration),
              interpolator_(interpolator)
    { }

    interpolation::InterpolatorReference interpolator_;
};

} // lightdb::physical

#endif //LIGHTDB_INTERPOLATEOPERATORS_H
