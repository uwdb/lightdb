#include "Algebra.h"
#include "LightField.h"

using namespace lightdb::catalog;

namespace lightdb::logical {
    LightFieldReference Scan(const std::string &name) {
        return Scan(Catalog::instance(), name);
    }

    LightFieldReference Scan(const catalog::Catalog &catalog, const std::string &name) {
        return catalog.get(name);
    }

    LightFieldReference Algebra::Select(const Volume &volume)
    {
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    LightFieldReference Algebra::Select(const SpatiotemporalDimension dimension, const SpatiotemporalRange &range)
    {
        Volume volume(this_->volume().bounding());
        volume.set(dimension, range);
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    LightFieldReference Algebra::Select(const ThetaRange &range)
    {
        Volume volume(this_->volume().bounding());
        volume.theta(range);
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    LightFieldReference Algebra::Select(const PhiRange &range)
    {
        Volume volume(this_->volume().bounding());
        volume.phi(range);
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    void Algebra::Store(const std::string &name)
    {
        throw NotImplementedError(); //TODO
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

    LightFieldReference Algebra::Partition(const Dimension dimension, const number interval) {
        return LightFieldReference::make<logical::PartitionedLightField>(this_, dimension, interval);

    }

    LightFieldReference Algebra::Interpolate(const Dimension dimension,
                                             const interpolation::interpolator& interpolator) {
        return LightFieldReference::make<logical::InterpolatedLightField>(this_, dimension, interpolator);
    }

    LightFieldReference Algebra::Discretize(const GeometryReference& geometry) {
        return LightFieldReference::make<logical::DiscretizedLightField>(this_, geometry);
    }

    LightFieldReference Algebra::Discretize(const Dimension dimension, const number interval) {
        return Discretize(GeometryReference::make<IntervalGeometry>(dimension, interval));
    }

    LightFieldReference Algebra::Map(FunctorReference functor) {
        return LightFieldReference::make<logical::TransformedLightField>(this_, functor);
    }

} // namespace lightdb::logical