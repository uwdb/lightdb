#ifndef LIGHTDB_MODEL_H
#define LIGHTDB_MODEL_H

#include "LightField.h"
#include "Algebra.h"
#include "Geometry.h"
#include "Color.h"
#include "Interpolation.h"
#include "Encoding.h"
#include "Catalog.h"
#include "Functor.h"
#include "Visitor.h"
#include "Ffmpeg.h"
#include "functional.h"
#include "reference.h"
#include <memory>
#include <utility>
#include <vector>
#include <set>
#include <numeric>
#include <optional>
#include <stdexcept>

namespace lightdb::logical {
    class ConstantLightField : public LightField {
    public:
        ConstantLightField(const Color &color, const Volume &volume)
                : LightField({}, volume, color.colorSpace()), color_(color) { }

        static LightFieldReference create(const Color &color,
                                          const Volume &volume = Volume::limits()) {
            return LightFieldReference::make<ConstantLightField>(color, volume);
        }

        const Color &color() const { return color_; }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<ConstantLightField>(visitor); }

    private:
        const Color &color_;
    };

    class CompositeLightField : public LightField {
    public:
        explicit CompositeLightField(const std::vector<LightFieldReference> &lightfields)
                : LightField(asserts::CHECK_NONOVERLAPPING(asserts::CHECK_NONEMPTY(lightfields)),
                             CompositeVolume{functional::flatmap<std::vector<Volume>>(
                                     lightfields.begin(),
                                             lightfields.end(),
                                             [](const auto &l) {
                                                 return l->volume().components(); })},
                             std::all_of(lightfields.begin(),
                                         lightfields.end(),
                                         [this, lightfields](auto &l) {
                                             return l->colorSpace() == lightfields[0]->colorSpace(); })
                             ? lightfields[0]->colorSpace()
                             : UnknownColorSpace::instance())
        { }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<CompositeLightField>(visitor); }
    };

    class PartitionedLightField : public LightField {
    public:
        //TODO should we just have a positive_rational type?
        PartitionedLightField(const LightFieldReference &source,
                              const Dimension dimension,
                              const number &interval)
                : LightField(source,
                             CompositeVolume{functional::flatmap<std::vector<Volume>>(
                                             source->volume().components().begin(),
                                             source->volume().components().end(),
                                             [dimension, interval](auto &volume) {
                                                 return (std::vector<Volume>)volume.partition(
                                                         dimension, asserts::CHECK_POSITIVE(interval)); })}),
                  dimension_(dimension), interval_(asserts::CHECK_POSITIVE(interval)) { }

        Dimension dimension() const { return dimension_; }
        number interval() const { return interval_; }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<PartitionedLightField>(visitor); }

    private:
        const Dimension dimension_;
        const number interval_;
    };

    class SubsetLightField : public LightField {
    public:
        SubsetLightField(const LightFieldReference &lightfield, const Volume &volume)
                : LightField(lightfield,
                             CompositeVolume{functional::transform_if<Volume>(
                                     lightfield->volume().components().begin(),
                                     lightfield->volume().components().end(),
                                     [&volume](const auto &v) {
                                         return v & volume; },
                                     [&volume](const auto &v) {
                                         return volume.has_nonempty_intersection(v); }),
                                 lightfield->volume().bounding() & volume})
        { }

        std::set<Dimension> dimensions() const {
            std::set<Dimension> values;

            std::copy_if(std::begin(Dimensions::All), std::end(Dimensions::All),
                         std::inserter(values, values.begin()),
                         [this](const auto &d) {
                             return volume().bounding().get(d) != functional::single(parents())->volume().bounding().get(d); });

            return values;
        }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<SubsetLightField>(visitor); }
    };

    class RotatedLightField : public LightField {
    public:
        RotatedLightField(const LightFieldReference &lightfield, const angle &theta, const angle &phi)
                : LightField(lightfield),
                  offset_{0, 0, 0, 0, theta, phi}
        { }

        const Point6D& offset() const { return offset_; }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<RotatedLightField>(visitor); }

    private:
        const Point6D offset_;
    };

    class DiscreteLightField : public LightField {
    public:
        const GeometryReference& geometry() const { return geometry_; }

    protected:
        explicit DiscreteLightField(const LightFieldReference &source,
                                    const GeometryReference &geometry)
                : DiscreteLightField(source, source->volume(), geometry)
        { }
        explicit DiscreteLightField(const LightFieldReference &source,
                                    const CompositeVolume &volume,
                                    const GeometryReference &geometry)
                : LightField(source), geometry_(geometry)
        { }
        explicit DiscreteLightField(const CompositeVolume &volume,
                                    const ColorSpace &colorSpace,
                                    const GeometryReference &geometry)
                : LightField({}, volume, colorSpace), geometry_(geometry)
        { }

        virtual bool defined_at(const Point6D &point) const {
            return geometry_->defined_at(point);
        }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<DiscreteLightField>(visitor); }

    private:
        const GeometryReference geometry_;
    };

    class DiscretizedLightField : public DiscreteLightField {
    public:
        DiscretizedLightField(const LightFieldReference &source, const GeometryReference &geometry)
                : DiscreteLightField(source, geometry)
        { }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<DiscretizedLightField>(visitor); }
    };

    class InterpolatedLightField : public LightField {
    public:
        InterpolatedLightField(const LightFieldReference &source,
                               const Dimension dimension,
                               const interpolation::InterpolatorReference &interpolator)
                : LightField(source),
                  interpolator_(interpolator) {}

        const interpolation::InterpolatorReference interpolator() const { return interpolator_; }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<InterpolatedLightField>(visitor); }

    private:
        const interpolation::InterpolatorReference interpolator_;
    };

    class TransformedLightField : public LightField {
    public:
        TransformedLightField(const LightFieldReference &source,
                              const functor::UnaryFunctorReference &functor)
                : LightField(source), functor_(functor)
        { }

        const functor::UnaryFunctorReference& functor() const { return functor_; };

        void accept(LightFieldVisitor &visitor) override { LightField::accept<TransformedLightField>(visitor); }

    private:
        const functor::UnaryFunctorReference functor_;
    };

    class SubqueriedLightField : public LightField {
    public:
        SubqueriedLightField(const LightFieldReference &source,
                             std::function<LightFieldReference(LightFieldReference)> subquery)
                : LightField(source), subquery_(std::move(subquery))
        { }

        const std::function<LightFieldReference(LightFieldReference)>& subquery() const { return subquery_; };

        void accept(LightFieldVisitor &visitor) override { LightField::accept<SubqueriedLightField>(visitor); }

    private:
        const std::function<LightFieldReference(LightFieldReference)> subquery_;
    };

    class StreamBackedLightField {
    public:
        virtual const std::vector<catalog::Source> sources() const = 0;
    };

    class ScannedLightField : public LightField, public StreamBackedLightField {
    public:
        explicit ScannedLightField(catalog::Entry entry)
                : LightField({}, entry.volume(), entry.colorSpace()), entry_(std::move(entry)) { }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<ScannedLightField>(visitor); }

        const catalog::Entry& entry() const noexcept { return entry_; }
        const std::vector<catalog::Source> sources() const override { return entry().sources(); }

    private:
        const catalog::Entry entry_;
    };

    class ExternalLightField : public LightField, public StreamBackedLightField, public OptionContainer<> {
    public:
        ExternalLightField(const std::filesystem::path &filename,
                           const catalog::ExternalEntry &entry,
                           const ColorSpace &colorSpace,
                           lightdb::options<> options)
                : LightField({}, entry.sources().front().volume(), colorSpace),
                  entry_(entry),
                  options_(std::move(options)) {
            CHECK_EQ(entry_.sources().size(), 1) << "External TLFs do not currently support >1 source";
        }

        inline const catalog::Source& source() const noexcept { return entry_.sources().front(); }
        inline const Codec& codec() const noexcept { return entry_.sources().front().codec(); }
        inline const std::vector<catalog::Source> sources() const override { return entry_.sources(); }
        inline const lightdb::options<>& options() const override {return options_; }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<ExternalLightField>(visitor); }

    private:
        const catalog::ExternalEntry entry_;
        const lightdb::options<> options_;
    };

    class EncodedLightField : public LightField, public OptionContainer<> {
    public:
        EncodedLightField(LightFieldReference parent,
                          Codec codec,
                          const Volume &volume,
                          const ColorSpace &colorSpace,
                          lightdb::options<> options)
                : LightField({parent}, volume, colorSpace),
                  codec_(std::move(codec)),
                  options_(std::move(options))
        { }

        const lightdb::options<>& options() const override {return options_; }
        const Codec& codec() const { return codec_; }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<EncodedLightField>(visitor); }

    private:
        const Codec codec_;
        const lightdb::options<> options_;
    };

    class StoredLightField : public LightField {
    public:
        explicit StoredLightField(const LightFieldReference &source,
                                  std::string name,
                                  const catalog::Catalog &catalog,
                                  std::optional<GeometryReference> geometry={},
                                  Codec codec=Codec::hevc())
                : LightField(source),
                  name_(std::move(name)),
                  codec_(std::move(codec)),
                  geometry_(std::move(geometry)),
                  catalog_(catalog)
        { }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<StoredLightField>(visitor); }

        inline const std::string& name() const { return name_; }
        inline const catalog::Catalog& catalog() const {return catalog_; }
        inline const std::optional<GeometryReference>& geometry() const { return geometry_; }
        inline const Codec& codec() const { return codec_; }

    private:
        const std::string name_;
        const Codec codec_;
        const std::optional<GeometryReference> geometry_;
        const catalog::Catalog catalog_;
    };

    class SavedLightField : public LightField {
    public:
        SavedLightField(const LightFieldReference &parent,
                        std::filesystem::path filename,
                        Codec codec=Codec::hevc(),
                        std::optional<GeometryReference> geometry={})
                : LightField(parent),
                  filename_(std::move(filename)),
                  codec_(std::move(codec)),
                  geometry_(std::move(geometry)) { }

        inline const std::filesystem::path& filename() const noexcept { return filename_; }
        inline const std::optional<GeometryReference>& geometry() const noexcept { return geometry_; }
        inline const Codec& codec() const { return codec_; }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<SavedLightField>(visitor); }

    private:
        const std::filesystem::path filename_;
        const Codec codec_;
        const std::optional<GeometryReference> geometry_;
    };

    class SunkLightField : public LightField {
    public:
        explicit SunkLightField(const LightFieldReference &source)
                : LightField(source)
        { }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<SunkLightField>(visitor); }
    };

} // namespace lightdb::logical

#endif //LIGHTDB_MODEL_H
