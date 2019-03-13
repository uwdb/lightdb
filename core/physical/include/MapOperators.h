#ifndef LIGHTDB_MAPOPERATORS_H
#define LIGHTDB_MAPOPERATORS_H
#include <utility>

#include "PhysicalOperators.h"
#include "EncodeOperators.h"
#include "Functor.h"

namespace lightdb::physical {

//TODO CPUMap and GPU map can be combined into one templated class
class GPUMap: public GPUUnaryOperator {
public:
    GPUMap(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent,
           const functor::unaryfunctor &transform)
            : GPUUnaryOperator(logical, parent, runtime::make<Runtime>(*this)),
              transform_(transform)
    { }

    const functor::unaryfunctor transform() const { return transform_; }

private:

    class Runtime: public GPUUnaryOperator::Runtime<GPUMap, GPUDecodedFrameData> {
    public:
        explicit Runtime(GPUMap &physical)
            : GPUUnaryOperator::Runtime<GPUMap, GPUDecodedFrameData>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterator() != iterator().eos()) {
                auto input = iterator()++;

                auto &transform = physical().transform()(DeviceType::GPU);
                auto output = transform(input);
                return dynamic_cast<MaterializedLightField&>(*output).ref();
            } else
                return {};
        }
    };

    const functor::unaryfunctor transform_;
};

class CPUMap: public PhysicalLightField {
public:
    CPUMap(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent,
           const functor::unaryfunctor &transform)
            : PhysicalLightField(logical, {parent}, physical::DeviceType::CPU, runtime::make<Runtime>(*this)),
              transform_(transform)
    { }

    const functor::unaryfunctor transform() const { return transform_; }

private:
class Runtime: public runtime::UnaryRuntime<CPUMap, CPUDecodedFrameData> {
    public:
        explicit Runtime(CPUMap &physical)
            : runtime::UnaryRuntime<CPUMap, CPUDecodedFrameData>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterator() != iterator().eos()) {
                auto data = iterator()++;
                auto &transform = physical().transform()(DeviceType::CPU);

                auto output = transform(data);
                return dynamic_cast<MaterializedLightField&>(*output).ref();
            } else
                return {};
        }
    };

    const functor::unaryfunctor transform_;
};

} // lightdb::physical

#endif //LIGHTDB_MAPOPERATORS_H
