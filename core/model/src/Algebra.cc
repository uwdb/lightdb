#include "Algebra.h"
#include "LightField.h"

namespace lightdb::logical {
    LightFieldReference Algebra::Select(const Volume &volume)
    {
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    void Algebra::Store(const std::string &name)
    {
        throw NotImplementedError();
    }

    LightFieldReference Algebra::Union(const LightFieldReference other) {
        return LightFieldReference::make<logical::CompositeLightField>(std::vector<LightFieldReference>{this_, other});
    }

    LightFieldReference Algebra::Union(const std::vector<LightFieldReference> &others) {
        std::vector<LightFieldReference> combined{others};
        combined.push_back(this_);
        return LightFieldReference::make<logical::CompositeLightField>(combined);
    }

    LightFieldReference Algebra::Rotate(const angle theta, const angle phi) {
        return LightFieldReference::make<logical::RotatedLightField>(this_, theta, phi);
    }

    LightFieldReference Algebra::Partition(const Dimension dimension, const rational interval) {
        return LightFieldReference::make<logical::PartitionedLightField>(this_, dimension, interval);

    }

    LightFieldReference Algebra::Interpolate(const Dimension dimension,
                                             const interpolation::interpolator& interpolator) {
        return LightFieldReference::make<logical::InterpolatedLightField>(this_, dimension, interpolator);
    }

    LightFieldReference Algebra::Discretize(const Geometry &geometry) {
        return LightFieldReference::make<logical::DiscretizedLightField>(this_, geometry);
    }

    LightFieldReference Algebra::Discretize(const Dimension dimension, const rational interval) {
        return Discretize(IntervalGeometry(dimension, interval));
    }

    LightFieldReference Algebra::Map(functor& functor) {
        return LightFieldReference::make<logical::TransformedLightField>(this_, functor);
    }

} // namespace lightdb::logical