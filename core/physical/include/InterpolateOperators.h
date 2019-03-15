#ifndef LIGHTDB_INTERPOLATEOPERATORS_H
#define LIGHTDB_INTERPOLATEOPERATORS_H

#include <utility>

#include "PhysicalOperators.h"
#include "EncodeOperators.h"
#include "Functor.h"

namespace lightdb::physical {

class GPUInterpolate: public PhysicalLightField, public GPUOperator {
public:
    GPUInterpolate(const LightFieldReference &logical,
                   PhysicalLightFieldReference &parent,
                   const interpolation::InterpolatorReference &interpolator)
            : PhysicalLightField(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(parent),
              interpolator_(interpolator)
    { }

    interpolation::InterpolatorReference interpolator() const { return interpolator_; }

private:
    class Runtime: public runtime::GPUUnaryRuntime<GPUInterpolate, GPUDecodedFrameData> {
    public:
        explicit Runtime(GPUInterpolate &physical)
            : runtime::GPUUnaryRuntime<GPUInterpolate, GPUDecodedFrameData>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterator() != iterator().eos()) {
                auto input = iterator()++;

                auto i = MaterializedLightFieldReference::make<physical::InterpolatedGPUDecodedFrameData>(
                        input, physical().interpolator());
                return i;
            } else
                return {};
        }
    };

    interpolation::InterpolatorReference interpolator_;
};

} // lightdb::physical

#endif //LIGHTDB_INTERPOLATEOPERATORS_H
