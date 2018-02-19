#ifndef LIGHTDB_LIGHTFIELD_H
#define LIGHTDB_LIGHTFIELD_H

#include "Algebra.h"
#include "Geometry.h"
#include "Color.h"
#include "Interpolation.h"
#include "Encoding.h"
#include "Catalog.h"
#include "Ffmpeg.h"
#include "functional.h"
#include "reference.h"
#include <memory>
#include <utility>
#include <vector>
#include <numeric>
#include <optional>
#include <stdexcept>

namespace lightdb {
    class LightFieldReference;

    class LightField {
    protected:
        explicit LightField(const LightFieldReference &parent);
        explicit LightField(std::vector<LightFieldReference> &parents, const ColorSpace&);
        explicit LightField(const LightFieldReference &parent, const CompositeVolume&);
        explicit LightField(const LightFieldReference &parent, const CompositeVolume&, const ColorSpace&);
        explicit LightField(std::vector<LightFieldReference> parents, CompositeVolume volume,
                            const ColorSpace& colorSpace)
            : parents_(std::move(parents)), volume_(std::move(volume)), colorSpace_(colorSpace)
        { }

        virtual ~LightField() = default;

    public:
        template<typename T>
        inline const T& downcast() const {
            return dynamic_cast<const T&>(*this);
        }

        template<typename T>
        inline bool is() const {
            return dynamic_cast<const T*>(this) != nullptr;
        }

        template<typename T>
        inline const std::optional<std::reference_wrapper<const T>> try_downcast() const {
            return {dynamic_cast<const T&>(*this)};
        }

        virtual const std::vector<LightFieldReference> parents() const { return parents_; }

        virtual const ColorSpace colorSpace() const { return colorSpace_; }

        virtual const CompositeVolume& volume() const { return volume_; }

        inline std::string type() const { return typeid(*this).name(); }

    private:
        const std::vector<LightFieldReference> parents_;
        const CompositeVolume volume_;
        const ColorSpace colorSpace_;
    };

    //TODO using LightFieldReference = ::lightdb::shared_reference<LightField, logical::Algebra>;
    class LightFieldReference: public logical::Algebra {
    public:
        LightFieldReference(std::shared_ptr<LightField> lightfield)
            : Algebra(*this), pointer_(std::move(lightfield)), direct_(pointer_.get()) {}

        LightFieldReference(const LightFieldReference &reference)
            : Algebra(*this), pointer_(reference.pointer_), direct_(pointer_.get()) {}

        inline operator const LightField &() const {
            return *pointer_;
        }

        inline LightField *operator->() const {
            return pointer_.get();
        }

        template<typename LightField>
        inline LightField *operator->() const {
            return pointer_.get();
        }

        inline LightField &operator*() const {
            return *pointer_;
        }

        inline std::string type() const {
            return pointer_.get()->type();
        }

        explicit operator const std::shared_ptr<LightField>() const {
            return pointer_;
        }

        template<typename T, typename... _Args>
        inline static LightFieldReference
        make(_Args &&... args) {
            return static_cast<std::shared_ptr<LightField>>(std::make_shared<T>(args...));
        }

    private:
        const std::shared_ptr<LightField> pointer_;
        const LightField *direct_; // TODO Useful for debugging, not exposed
    };

    namespace logical {
        class ConstantLightField : public LightField {
        public:
            static LightFieldReference create(const Color &color,
                                              const Volume &volume = Volume::VolumeMax) {
                return std::shared_ptr<LightField>(new ConstantLightField(color, volume));
            }

            const Color &color() const { return color_; }

        protected:
            ConstantLightField(const Color &color, const Volume &volume)
                : LightField({}, volume, color.colorSpace()), color_(color) { }

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
                                 : UnknownColorSpace::Instance)
            { }
        };

        class PartitionedLightField : public LightField {
        public:
            //TODO should we just have a positive_rational class?
            PartitionedLightField(const LightFieldReference &source,
                                  const Dimension dimension,
                                  const rational &interval)
                    : LightField(source,
                                 CompositeVolume{functional::flatmap<std::vector<Volume>>(
                                     source->volume().components().begin(),
                                     source->volume().components().end(),
                                     [dimension, interval](auto &volume) {
                                         return volume.partition(dimension, asserts::CHECK_POSITIVE(interval)); })}),
                      dimension_(dimension), interval_(asserts::CHECK_POSITIVE(interval)) { }

            Dimension dimension() const { return dimension_; }
            rational interval() const { return interval_; }

        private:
            const Dimension dimension_;
            const rational interval_;
        };

        class SubsetLightField : public LightField {
        public:
            SubsetLightField(const LightFieldReference &lightfield, const Volume &volume)
                    : LightField(lightfield,
                                 CompositeVolume{functional::transform_if<Volume>(
                                     lightfield->volume().components().begin(),
                                     lightfield->volume().components().end(),
                                     [volume](auto &v) { return v | volume; },
                                     [](auto &v) { return !v.is_point(); })})
            { }
        };

        class RotatedLightField : public LightField {
        public:
            RotatedLightField(const LightFieldReference &lightfield, const angle &theta, const angle &phi)
                    : LightField(lightfield,
                                 CompositeVolume{functional::transform<Volume>(
                                     lightfield->volume().components().begin(),
                                     lightfield->volume().components().end(),
                                     [this](auto &v) {
                                         return v.translate(offset_); })}),
                      offset_{0, 0, 0, 0, theta, phi} {}

            const Point6D& offset() const { return offset_; }

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

        private:
            const GeometryReference geometry_;
        };

        class DiscretizedLightField : public DiscreteLightField {
        public:
            DiscretizedLightField(const LightFieldReference &source, const GeometryReference &geometry)
                : DiscreteLightField(source, geometry)
            { }
        };

        class InterpolatedLightField : public LightField {
        public:
            InterpolatedLightField(const LightFieldReference &source,
                                   const Dimension dimension,
                                   const interpolation::interpolator &interpolator)
                    : LightField(source),
                      interpolator_(interpolator) {}

            const interpolation::interpolator& interpolator() const { return interpolator_; }

        private:
            const interpolation::interpolator &interpolator_;
        };

        class TransformedLightField : public LightField {
        public:
            TransformedLightField(const LightFieldReference &source,
                                  const FunctorReference &functor)
                    : LightField(source), functor_(functor)
            { }

            const FunctorReference& functor() const { return functor_; };

        private:
            const FunctorReference functor_;
        };

        class ScannedLightField : public LightField {
        public:
            explicit ScannedLightField(catalog::Catalog::Metadata metadata)
                : LightField({}, metadata.volume(), metadata.colorSpace()), metadata_(std::move(metadata)) { }

        private:
            const catalog::Catalog::Metadata metadata_;
        };

        class ExternalLightField : public LightField {
        public:
            ExternalLightField(std::string uri, const Volume &volume,
                               const ColorSpace &colorSpace, const Geometry &geometry)
                    : LightField({}, volume, colorSpace),
                      uri_(std::move(uri)), colorSpace_(colorSpace), geometry_(geometry) { }

        private:
            const std::string uri_;
            const ColorSpace &colorSpace_;
            const Geometry &geometry_;
        };

        //TODO These all moved to physical algebra, obvs
        //TODO think now that I've moved geometry to physical algebra, this whole class should die
        class PanoramicLightField : public DiscreteLightField {
        public:
            PanoramicLightField(const Point3D &point,
                                const TemporalRange &range,
                                const AngularRange &theta = AngularRange::ThetaMax,
                                const AngularRange &phi = AngularRange::PhiMax)
                : DiscreteLightField(point.ToVolume(range, theta, phi), YUVColorSpace::Instance,
                                     GeometryReference::make<EquirectangularGeometry>()),
                  point_(point)
 //                   : DiscreteLightField(EquirectangularGeometry::Instance, point.ToVolume(range, theta, phi)),
   //                   point_(point)
            //, volume_{point.ToVolume(range, theta, phi)} {}
            { }
            explicit PanoramicLightField(const TemporalRange &range)
                    : PanoramicLightField(Point3D{0, 0, 0}, range) {}

        protected:
            bool defined_at(const Point6D &point) const override {
                return volume().bounding().Contains(point) && EquirectangularGeometry::Instance.defined_at(point);
            }

        private:
            const Point3D point_;
        };

        class PanoramicVideoLightField : public PanoramicLightField, public SingletonFileEncodedLightField {
        public:
            explicit PanoramicVideoLightField(const std::string &filename,
                                              const AngularRange &theta = AngularRange::ThetaMax,
                                              const AngularRange &phi = AngularRange::PhiMax) //TODO drop filename; see below
                    : PanoramicVideoLightField(filename, Point3D::Zero, theta, phi) {}

            PanoramicVideoLightField(const std::string &filename,
                                     const Point3D &point,
                                     const AngularRange &theta = AngularRange::ThetaMax,
                                     const AngularRange &phi = AngularRange::PhiMax) //TODO drop filename; see below
                    : PanoramicLightField(point, {0, duration()}, theta, phi),
                      SingletonFileEncodedLightField(filename,
                                                     (Volume) PanoramicLightField::volume()), //TODO no need to duplicate filename here and in singleton
                      geometry_(Dimension::Time, framerate()) {}

            //const std::vector<LightFieldReference> parents() const override { return {}; }

            //const ColorSpace colorSpace() const override { return YUVColorSpace::Instance; } //TODO pull from file
            //TODO remove both of these
            inline rational framerate() const { return rational(30, 1); } //TODO hardcoded...

            inline size_t duration() const { return 99; } //TODO remove hardcoded value
            inline utility::StreamMetadata metadata() const {
                return (metadata_.has_value()
                        ? metadata_
                        : (metadata_ = utility::StreamMetadata(filename(), 0, true))).value();
            }

        protected:
            bool defined_at(const Point6D &point) const override {
                return geometry_.defined_at(point) &&
                       PanoramicLightField::defined_at(point);
            }

        private:
            //TODO drop filename after adding StreamDecodeReader in Physical.cc
            // Can make SingletonEncodedLightField be a replacement for this class; one for in-memory, one for on-disk
            const IntervalGeometry geometry_;
            mutable std::optional<utility::StreamMetadata> metadata_;
        };

        class PlanarTiledVideoLightField : public DiscreteLightField, public SingletonFileEncodedLightField {
        public:
            PlanarTiledVideoLightField(const std::string &filename,
                                       const Volume &volume,
                                       const size_t rows, const size_t columns)
                    : DiscreteLightField(volume, YUVColorSpace::Instance,
                                         GeometryReference::make<IntervalGeometry>(Dimension::Time, framerate())),
                      SingletonFileEncodedLightField(filename,
                                                     volume), //TODO no need to duplicate filename here and in singleton
                      //volume_(volume),
                      rows_(rows), columns_(columns) {}

            //const std::vector<LightFieldReference> parents() const override { return {}; }

            //const ColorSpace colorSpace() const override { return YUVColorSpace::Instance; } //TODO pull from file
            //TODO remove both of these
            inline rational framerate() const { return rational(30, 1); } //TODO hardcoded...

            inline size_t duration() const { return 20; } //TODO remove hardcoded value
            inline utility::StreamMetadata metadata() const {
                return (metadata_.has_value()
                        ? metadata_
                        : (metadata_ = utility::StreamMetadata(filename(), 0, true))).value();
            }

        private:
            //TODO drop filename after adding StreamDecodeReader in Physical.cc
            // Can make SingletonEncodedLightField be a replacement for this class; one for in-memory, one for on-disk, one for streaming
            //const Volume volume_;
            const size_t rows_, columns_;
            mutable std::optional<utility::StreamMetadata> metadata_;
        };

    } // namespace logical
} // namespace lightdb
#endif //LIGHTDB_LIGHTFIELD_H
