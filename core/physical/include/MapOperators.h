#ifndef LIGHTDB_MAPOPERATORS_H
#define LIGHTDB_MAPOPERATORS_H
#include <utility>

#include "PhysicalOperators.h"
#include "EncodeOperators.h"
#include "Functor.h"

namespace lightdb::physical {

class GPUMap: public GPUUnaryOperator<GPUDecodedFrameData> {
public:
    GPUMap(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent,
           const functor::unaryfunctor &transform)
            : GPUMap(logical, parent, [this]() -> Configuration { return this->parent<GPUOperator>().configuration(); },
                     transform)
    { }

    GPUMap(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent,
           const Configuration &configuration,
           const functor::unaryfunctor &transform)
            : GPUMap(logical, parent, [configuration]() { return configuration; }, transform)
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(iterator() != iterator().eos()) {
            auto input = iterator()++;

            auto &transform = transform_(DeviceType::GPU);
            auto output = transform(input);
            auto &field = static_cast<LightField&>(output);
            auto &data = dynamic_cast<MaterializedLightField&>(field);
            return static_cast<physical::MaterializedLightFieldReference>(data);
        } else
            return {};
    }

    const functor::unaryfunctor transform() const { return transform_; }

private:
    GPUMap(const LightFieldReference &logical,
           PhysicalLightFieldReference &parent,
           const std::function<Configuration()> &configuration,
           const functor::unaryfunctor &transform)
            : GPUUnaryOperator(logical, parent, configuration),
              transform_(transform)
    { }

    const functor::unaryfunctor transform_;
};

    class CPUMap: public PhysicalLightField {
    public:
        CPUMap(const LightFieldReference &logical,
               PhysicalLightFieldReference &parent,
               const functor::unaryfunctor &transform)
                : PhysicalLightField(logical, {parent}, physical::DeviceType::CPU),
                  transform_(transform)
        { }


        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterators()[0] != iterators()[0].eos()) {
                auto input = iterators()[0]++;
                auto &data = input.downcast<CPUDecodedFrameData>();
                auto &transform = transform_(DeviceType::CPU);

                auto result = transform(data);
                auto &field = static_cast<LightField&>(result);
                auto &output = dynamic_cast<MaterializedLightField&>(field);
                return static_cast<physical::MaterializedLightFieldReference>(output);
            } else
                return {};
        }

        const functor::unaryfunctor transform() const { return transform_; }

    private:
        const functor::unaryfunctor transform_;
    };
} // lightdb::physical

#endif //LIGHTDB_MAPOPERATORS_H
