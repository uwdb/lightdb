#include "Algebra.h"
#include "Model.h"

using namespace lightdb::catalog;

namespace lightdb::logical {
    LightFieldReference Scan(const std::string &name) {
        return Scan(Catalog::instance(), name);
    }

    LightFieldReference Scan(const catalog::Catalog &catalog, const std::string &name) {
        return catalog.get(name);
    }

    LightFieldReference Load(const std::filesystem::path& filename, const lightdb::options<>& options) {
        auto codec_name = std::any_cast<std::string>(options.get("Codec").value_or(
                std::any{filename.extension().string()}));
        auto codec = Codec::get(codec_name);

        if(codec.has_value())
            return Load(filename, codec.value());
        else
            throw InvalidArgumentError("Could not infer codec from options or uri", "options");
    }

    LightFieldReference Load(const std::filesystem::path &filename, const Codec& codec, const Volume& volume,
                             const ColorSpace &colorSpace, const GeometryReference &geometry,
                             const lightdb::options<>& options) {
        return LightFieldReference::make<ExternalLightField>(filename, codec, volume, colorSpace, geometry, options);
    }

    LightFieldReference Algebra::Save(const std::filesystem::path &filename) {
        return LightFieldReference::make<SavedLightField>(this_, filename);
    }

    LightFieldReference Algebra::Select(const Volume &volume) {
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    LightFieldReference Algebra::Select(const SpatiotemporalDimension dimension, const SpatiotemporalRange &range) {
        Volume volume(this_->volume().bounding());
        volume.set(dimension, range);
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    LightFieldReference Algebra::Select(const ThetaRange &range) {
        Volume volume(this_->volume().bounding());
        volume.theta(range);
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    LightFieldReference Algebra::Select(const PhiRange &range) {
        Volume volume(this_->volume().bounding());
        volume.phi(range);
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    LightFieldReference Algebra::Select(const TemporalRange &range) {
        Volume volume(this_->volume().bounding());
        volume.t(range);
        return LightFieldReference::make<SubsetLightField>(this_, volume);
    }

    LightFieldReference Algebra::Store(const std::string &name, const Codec &codec) {
        return LightFieldReference::make<StoredLightField>(this_, name, codec);
    }

    LightFieldReference Algebra::Sink() {
        return LightFieldReference::make<SunkLightField>(this_);
    }

    LightFieldReference Algebra::Encode(const Codec &codec, const options<> &options) {
        return LightFieldReference::make<logical::EncodedLightField>(
                this_, codec, this_->volume().bounding(), this_->colorSpace(), options);
    }

    LightFieldReference Algebra::Union(const LightFieldReference other) {
        return LightFieldReference::make<logical::CompositeLightField>(std::vector<LightFieldReference>{this_, other});
    }

    LightFieldReference Algebra::Union(std::vector<LightFieldReference> fields) {
        fields.push_back(this_);
        return LightFieldReference::make<logical::CompositeLightField>(fields);
    }

    LightFieldReference Algebra::Rotate(const angle theta, const angle phi) {
        return LightFieldReference::make<logical::RotatedLightField>(this_, theta, phi);
    }

    LightFieldReference Algebra::Partition(const Dimension dimension, const number& interval) {
        return LightFieldReference::make<logical::PartitionedLightField>(this_, dimension, interval);

    }

    LightFieldReference Algebra::Interpolate(const Dimension dimension,
                                             interpolation::InterpolatorReference interpolator) {
        return LightFieldReference::make<logical::InterpolatedLightField>(this_, dimension, interpolator);
    }

    LightFieldReference Algebra::Discretize(const GeometryReference& geometry) {
        return LightFieldReference::make<logical::DiscretizedLightField>(this_, geometry);
    }

    LightFieldReference Algebra::Discretize(const Dimension dimension, const number& interval) {
        return Discretize(GeometryReference::make<IntervalGeometry>(dimension, interval));
    }

    LightFieldReference Algebra::Map(functor::UnaryFunctorReference functor) {
        return LightFieldReference::make<logical::TransformedLightField>(this_, functor);
    }

    LightFieldReference Algebra::Subquery(const std::function<LightFieldReference(LightFieldReference)> &subquery) {
        return LightFieldReference::make<logical::SubqueriedLightField>(this_, subquery);
    }

} // namespace lightdb::logical