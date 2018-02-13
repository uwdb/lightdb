#ifndef LIGHTDB_LIGHTFIELD_H
#define LIGHTDB_LIGHTFIELD_H

#include "Geometry.h"
#include "Color.h"
#include "Interpolation.h"
#include "Encoding.h"
#include "Ffmpeg.h"
#include <memory>
#include <utility>
#include <vector>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <functional.h>

class LightFieldReference;
namespace lightdb {
    template<typename ColorSpace> class functor;
}

class LightField {
protected:
    LightField() = default;
    virtual ~LightField() = default;

public:
    virtual const std::vector<LightFieldReference> parents() const = 0;
    virtual const ColorSpace colorSpace() const = 0;
    virtual const CompositeVolume volume() const = 0;
    inline std::string type() const { return typeid(*this).name(); }
};

class LightFieldReference {
public:
    LightFieldReference(std::shared_ptr<LightField> lightfield)
        : pointer_(std::move(lightfield)), direct_(pointer_.get())
    { }

    LightFieldReference(const LightFieldReference &reference)
        : pointer_(reference.pointer_), direct_(pointer_.get())
    { }

    inline operator const LightField&() const {
        return *pointer_;
    }

    inline LightField* operator->() const {
        return pointer_.get();
    }

    inline LightField& operator*() const {
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
    make(_Args&&... args) {
        return static_cast<std::shared_ptr<LightField>>(std::make_shared<T>(args...));
    }

private:
    const std::shared_ptr<LightField> pointer_;
    const LightField *direct_; // TODO Useful for debugging, not exposed
};

class ConstantLightField: public LightField {
public:
    static LightFieldReference create(const Color &color,
                                      const Volume &volume=Volume::VolumeMax) {
        return std::shared_ptr<LightField>(new ConstantLightField(color, volume));
    }

    const std::vector<LightFieldReference> parents() const override { return {}; }
    const CompositeVolume volume() const override { return volume_; }
    const Color& color() const { return color_; }
    const ColorSpace colorSpace() const override { return color().colorSpace(); }

protected:
    ConstantLightField(const Color& color, const Volume &volume)
        : color_(color), volume_(volume)
    { }

private:
    const Color &color_;
    const Volume volume_;
};

class CompositeLightField: public LightField {
public:
    //TODO ensure that lightfields don't overlap
    explicit CompositeLightField(std::vector<LightFieldReference> lightfields)
        : lightfields_(std::move(lightdb::errors::CHECK_NONEMPTY(lightfields)))
    { }

    const std::vector<LightFieldReference> parents() const override { return lightfields_; }
    const ColorSpace colorSpace() const override {
        return std::all_of(lightfields_.begin(), lightfields_.end(),
                           [this](auto &l) { return l->colorSpace() == lightfields_[0]->colorSpace(); })
            ? lightfields_[0]->colorSpace()
            : UnknownColorSpace::Instance;
    }
    const CompositeVolume volume() const override {
        return {lightdb::functional::flatmap<std::vector<Volume>>(
                lightfields_.begin(),
                lightfields_.end(),
                [](auto &l) { return l->volume().components(); })};
    }

private:
    const std::vector<LightFieldReference> lightfields_;
};

class PartitionedLightField: public LightField {
public:
    PartitionedLightField(const LightFieldReference &source,
                          const Dimension dimension,
                          const lightdb::rational &interval)
        : source_(source), dimension_(dimension), interval_(interval)
    { }

    const std::vector<LightFieldReference> parents() const override { return {source_}; }
    const ColorSpace colorSpace() const override { return source_->colorSpace(); }
    const CompositeVolume volume() const override {
        return {lightdb::functional::flatmap<std::vector<Volume>>(
                source_->volume().components().begin(),
                source_->volume().components().end(),
                [this](auto &volume) { return volume.partition(dimension_, interval_); })};
    }

    Dimension dimension() const { return dimension_; }
    lightdb::rational interval() const { return interval_; }

private:
    const LightFieldReference source_;
    const Dimension dimension_;
    const lightdb::rational interval_;
};

class SubsetLightField: public LightField {
public:
    SubsetLightField(const LightFieldReference &lightfield, const Volume &volume)
        : lightfield_(lightfield), volume_(volume | lightfield->volume().bounding())
    { }

    const std::vector<LightFieldReference> parents() const override { return {lightfield_}; }
    const ColorSpace colorSpace() const override { return lightfield_->colorSpace(); }
    const CompositeVolume volume() const override {
        return CompositeVolume{
                lightdb::functional::transform_if<Volume>(
                    lightfield_->volume().components().begin(),
                    lightfield_->volume().components().end(),
                    [this](auto &v) { return v | volume_; },
                    [](auto &v) { return !v.is_point(); })
        };
    }

private:
    const LightFieldReference lightfield_;
    const Volume volume_;
};

class RotatedLightField: public LightField {
public:
    RotatedLightField(const LightFieldReference &lightfield, const angle &theta, const angle &phi)
            : lightfield_(lightfield), offset_{0, 0, 0, 0, theta, phi}
    { }

    const std::vector<LightFieldReference> parents() const override { return {lightfield_}; }
    const ColorSpace colorSpace() const override { return lightfield_->colorSpace(); }
    const CompositeVolume volume() const override {
        return {lightdb::functional::transform<Volume>(
                    lightfield_->volume().components().begin(),
                    lightfield_->volume().components().end(),
                    [this](auto &v) { return v.translate(offset_); })};
    }

private:
    const LightFieldReference lightfield_;
    const Point6D offset_;
};

class DiscreteLightField: public LightField {
public:
    explicit DiscreteLightField(const Geometry &geometry)
        : DiscreteLightField(std::bind(&Geometry::defined_at, &geometry, std::placeholders::_1))
    { }

    explicit DiscreteLightField(std::function<bool(const Point6D &)> predicate)
            : predicate_(std::move(predicate))
    { }

protected:
    virtual bool defined_at(const Point6D &point) const {
        return predicate_(point);
    }

private:
    const std::function<bool(const Point6D&)> predicate_;
};

class DiscretizedLightField: public DiscreteLightField {
public:
    DiscretizedLightField(const LightFieldReference &source, const Geometry &geometry)
        : DiscreteLightField(geometry), source_(source)
    { }

    const std::vector<LightFieldReference> parents() const override { return {source_}; }
    const CompositeVolume volume() const override { return source_->volume(); }
    const ColorSpace colorSpace() const override { return source_->colorSpace(); }

private:
    const LightFieldReference source_;
};

class InterpolatedLightField: public LightField {
public:
    InterpolatedLightField(const LightFieldReference &source,
                           const Dimension dimension,
                           lightdb::interpolation::interpolator<YUVColorSpace> interpolator)
        : source_(source), interpolator_(std::move(interpolator))
    { }

    const std::vector<LightFieldReference> parents() const override { return {source_}; }
    const CompositeVolume volume() const override { return source_->volume(); }
    const ColorSpace colorSpace() const override { return source_->colorSpace(); }

private:
    const LightFieldReference source_;
    const lightdb::interpolation::interpolator<YUVColorSpace> interpolator_;
};

class TransformedLightField: public LightField {
public:
    TransformedLightField(const LightFieldReference &source,
                           const lightdb::functor<YUVColorSpace> &functor)
        : source_(source), functor_(functor)
    { }

    const lightdb::functor<YUVColorSpace> &functor() const { return functor_; };

    //TODO just add a DecoratedLightField class that does all of this, rather than repeating it over and over
    const std::vector<LightFieldReference> parents() const override { return {source_}; }
    const CompositeVolume volume() const override { return source_->volume(); }
    const ::ColorSpace colorSpace() const override { return source_->colorSpace(); }

private:
    const LightFieldReference source_;
    const lightdb::functor<YUVColorSpace> &functor_;
};

//TODO think now that I've moved geometry to physical algebra, this whole class should die
class PanoramicLightField: public DiscreteLightField {
public:
    PanoramicLightField(const Point3D &point,
                        const TemporalRange &range,
                        const AngularRange& theta=AngularRange::ThetaMax,
                        const AngularRange& phi=AngularRange::PhiMax)
        : DiscreteLightField(EquirectangularGeometry::Instance),
          point_(point), volume_{point.ToVolume(range, theta, phi)}
    { }

    explicit PanoramicLightField(const TemporalRange &range)
        : PanoramicLightField(Point3D{0, 0, 0}, range)
    { }

    const CompositeVolume volume() const override { return volume_; }

protected:
    bool defined_at(const Point6D &point) const override {
        return volume_.Contains(point) && EquirectangularGeometry::Instance.defined_at(point);
    }

private:
    const Point3D point_;
    const Volume volume_;
};

class PanoramicVideoLightField: public PanoramicLightField, public lightdb::SingletonFileEncodedLightField {
public:
    explicit PanoramicVideoLightField(const std::string &filename,
                                      const AngularRange& theta=AngularRange::ThetaMax,
                                      const AngularRange& phi=AngularRange::PhiMax) //TODO drop filename; see below
        : PanoramicVideoLightField(filename, Point3D::Zero, theta, phi)
    { }

    PanoramicVideoLightField(const std::string &filename,
                             const Point3D &point,
                             const AngularRange& theta=AngularRange::ThetaMax,
                             const AngularRange& phi=AngularRange::PhiMax) //TODO drop filename; see below
            : PanoramicLightField(point, {0, duration()}, theta, phi),
              SingletonFileEncodedLightField(filename, (Volume)PanoramicLightField::volume()), //TODO no need to duplicate filename here and in singleton
              geometry_(Dimension::Time, framerate())
    { }

    const CompositeVolume volume() const override { return PanoramicLightField::volume(); }
    const std::vector<LightFieldReference> parents() const override { return {}; }
    const ColorSpace colorSpace() const override { return YUVColorSpace::Instance; } //TODO pull from file
    //TODO remove both of these
    inline lightdb::rational framerate() const { return lightdb::rational(30, 1); } //TODO hardcoded...

    inline size_t duration() const { return 99; } //TODO remove hardcoded value
    inline lightdb::utility::StreamMetadata metadata() const { return (metadata_.has_value()
                                                                          ? metadata_
                                                                          : (metadata_ = lightdb::utility::StreamMetadata(filename(), 0, true))).value(); }
protected:
    bool defined_at(const Point6D &point) const override {
        return geometry_.defined_at(point) &&
               PanoramicLightField::defined_at(point);
    }

private:
    //TODO drop filename after adding StreamDecodeReader in Physical.cc
    // Can make SingletonEncodedLightField be a replacement for this class; one for in-memory, one for on-disk
    const IntervalGeometry geometry_;
    mutable std::optional<lightdb::utility::StreamMetadata> metadata_;
};

class PlanarTiledVideoLightField: public DiscreteLightField, public lightdb::SingletonFileEncodedLightField {
public:
    PlanarTiledVideoLightField(const std::string &filename,
                               const Volume &volume,
                               const size_t rows, const size_t columns)
            : DiscreteLightField(IntervalGeometry(Dimension::Time, framerate())),
              SingletonFileEncodedLightField(filename, volume), //TODO no need to duplicate filename here and in singleton
              volume_(volume),
              rows_(rows), columns_(columns)
    { }

    const CompositeVolume volume() const override { return volume_; }
    const std::vector<LightFieldReference> parents() const override { return {}; }
    const ColorSpace colorSpace() const override { return YUVColorSpace::Instance; } //TODO pull from file
    //TODO remove both of these
    inline lightdb::rational framerate() const { return lightdb::rational(30, 1); } //TODO hardcoded...

    inline size_t duration() const { return 20; } //TODO remove hardcoded value
    inline lightdb::utility::StreamMetadata metadata() const { return (metadata_.has_value()
                                                                           ? metadata_
                                                                           : (metadata_ = lightdb::utility::StreamMetadata(filename(), 0, true))).value(); }

private:
    //TODO drop filename after adding StreamDecodeReader in Physical.cc
    // Can make SingletonEncodedLightField be a replacement for this class; one for in-memory, one for on-disk, one for streaming
    const Volume volume_;
    const size_t rows_, columns_;
    mutable std::optional<lightdb::utility::StreamMetadata> metadata_;
};

#endif //LIGHTDB_LIGHTFIELD_H
