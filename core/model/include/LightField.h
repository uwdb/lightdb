#ifndef VISUALCLOUD_LIGHTFIELD_H
#define VISUALCLOUD_LIGHTFIELD_H

#include "Geometry.h"
#include "Color.h"
#include "Interpolation.h"
#include "Ffmpeg.h"
#include <memory>
#include <vector>
#include <numeric>
#include <optional>
#include <stdexcept>

template<typename ColorSpace>
class LightFieldReference;

template<typename ColorSpace>
class LightField { //: public LightField {
protected:
    LightField() { }
    virtual ~LightField() { }

public:
    virtual const std::vector<LightFieldReference<ColorSpace>> provenance() const = 0;
    virtual const typename ColorSpace::Color value(const Point6D &point) const = 0;
    virtual const ColorSpace colorSpace() const { return ColorSpace::Instance; }
    virtual const std::vector<Volume> volumes() const = 0;

    inline std::string type() const {
        return typeid(*this).name();
    }
};

template<typename ColorSpace>
class LightFieldReference {
public:
    LightFieldReference(const std::shared_ptr<LightField<ColorSpace>> lightfield)
        : pointer_(lightfield)
    { }

    LightFieldReference(LightField<ColorSpace> &&lightfield)
            : pointer_(nullptr)
    { }

    LightFieldReference(const LightFieldReference &reference)
            : pointer_(reference.pointer_)
    { }

    inline operator LightField<ColorSpace>&() const {
        return *pointer_;
    }

    inline LightField<ColorSpace>* operator->() const {
        return pointer_.get();
    }

    inline LightField<ColorSpace>& operator*() const {
        return *pointer_;
    }

    inline std::string type() const {
        return this->name();
    }

    template<typename T, typename... _Args>
    inline static LightFieldReference<ColorSpace>
    make(_Args&&... args) {
        return static_cast<std::shared_ptr<LightField<ColorSpace>>>(std::make_shared<T>(args...));
    }

private:
    const std::shared_ptr<LightField<ColorSpace>> pointer_;
};

template<typename ColorSpace>
class ConstantLightField: public LightField<ColorSpace> {
public:
    static LightFieldReference<ColorSpace> create(const typename ColorSpace::Color& color,
                                                  const Volume &volume=Volume::VolumeMax) {
        return std::shared_ptr<LightField<ColorSpace>>(new ConstantLightField<ColorSpace>(color, volume));
    }

    const typename ColorSpace::Color value(const Point6D &point) const override {
        return volume_.Contains(point) ? color_ : ColorSpace::Color::Null;
    }

    const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return {}; }
    const std::vector<Volume> volumes() const override { return {volume_}; }
    const ColorSpace colorSpace() const override { return colorSpace_; }

protected:
    ConstantLightField(const typename ColorSpace::Color& color, const Volume &volume)
            : colorSpace_(ColorSpace::Instance), color_(color), volume_(volume)
    { }

private:
    const ColorSpace colorSpace_;
    const typename ColorSpace::Color color_;
    const Volume volume_;
};

template<typename ColorSpace>
class CompositeLightField: public LightField<ColorSpace> {
public:
    CompositeLightField(const std::vector<LightFieldReference<ColorSpace>> &lightfields)
        : lightfields_(lightfields)
    { }

    const typename ColorSpace::Color value(const Point6D &point) const override {
        typename ColorSpace::Color color = ColorSpace::Color::Null;

        for(auto lightfield: lightfields_) {
            if((color = lightfield->value(point)) != ColorSpace::Color::Null)
                return color;
        }

        return ColorSpace::Color::Null;
    }

    const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return lightfields_; }
    const ColorSpace colorSpace() const override { return ColorSpace::Instance; } //TODO
    const std::vector<Volume> volumes() const override {
        return std::accumulate(lightfields_.begin(),
                               lightfields_.end(),
                               std::vector<Volume>(lightfields_.size() * 4),
                               [](auto &result, auto &field) {
                                   auto volumes = field->volumes();
                                   result.insert(result.end(), volumes.begin(), volumes.end());
                                   return result;
                               });
    }

private:
    const std::vector<LightFieldReference<ColorSpace>> lightfields_;
};

template<typename ColorSpace>
class PartitionedLightField: public LightField<ColorSpace> {
public:
    PartitionedLightField(const LightFieldReference<ColorSpace> &source,
                          const Dimension dimension, const visualcloud::rational &interval)
        : source_(source), dimension_(dimension), interval_(interval)
    { }

    inline const typename ColorSpace::Color value(const Point6D &point) const override {
        return source_->value(point);
    }

    const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return {source_}; }
    const ColorSpace colorSpace() const override { return ColorSpace::Instance; }
    const std::vector<Volume> volumes() const override {
        //TODO switch to iterator
        std::vector<Volume> result;
        for(auto &volume: source_->volumes()) {
            auto partitions = volume.partition(dimension_, interval_);
            result.insert(result.end(), partitions.begin(), partitions.end());
        }
        return result;
    }

    Dimension dimension() const { return dimension_; }
    visualcloud::rational interval() const { return interval_; }
    //const std::vector<LightFieldReference<ColorSpace>> partitions() const {
    //}

private:
    const LightFieldReference<ColorSpace> source_;
    const Dimension dimension_;
    const visualcloud::rational interval_;
};

template<typename ColorSpace>
class SubsetLightField: public LightField<ColorSpace> {
public:
    SubsetLightField(const LightFieldReference<ColorSpace> &lightfield, const Volume &volume)
        : lightfield_(lightfield), volume_(volume) //TODO don't we need to intersect volume with lightvield.volumes()?
    { }

    const typename ColorSpace::Color value(const Point6D &point) const override {
        return volume_.Contains(point)
            ? lightfield_->value(point)
            : ColorSpace::Color::Null;
    }

    const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return {lightfield_}; }
    const ColorSpace colorSpace() const override { return lightfield_->colorSpace(); }
    const std::vector<Volume> volumes() const override {
        //TODO clean this up
        std::vector<Volume> result;
        for(auto &volume: lightfield_->volumes()) {
            auto intersection = volume | volume_;
            if(!intersection.is_point())
                result.push_back(intersection);
        }
        return result;
    }

private:
    const LightFieldReference<ColorSpace> lightfield_;
    const Volume volume_;
};

template<typename ColorSpace>
class DiscreteLightField: public LightField<ColorSpace> {
public:
    DiscreteLightField(const Geometry &geometry)
        : DiscreteLightField(std::bind(&Geometry::defined_at, &geometry, std::placeholders::_1))
    { }

    DiscreteLightField(const std::function<bool(const Point6D&)> &predicate)
            : predicate_(predicate)
    { }

protected:
    virtual bool defined_at(const Point6D &point) const {
        return predicate_(point);
    }

private:
    const std::function<bool(const Point6D&)> predicate_;
};

template<typename ColorSpace>
class DiscretizedLightField: public DiscreteLightField<ColorSpace> {
public:
    DiscretizedLightField(const LightFieldReference<ColorSpace> &source, const Geometry &geometry)
        : DiscreteLightField<ColorSpace>(geometry), source_(source)
    { }
    virtual ~DiscretizedLightField() { }

    const typename ColorSpace::Color value(const Point6D &point) const override {
        return this->defined_at(point)
               ? source_->value(point)
               : ColorSpace::Color::Null;
    }

    const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return {source_}; }
    const std::vector<Volume> volumes() const override { return source_->volumes(); }
    const ColorSpace colorSpace() const override { return ColorSpace::Instance; }

private:
    const LightFieldReference<ColorSpace> source_;
};

template<typename ColorSpace>
class InterpolatedLightField: public LightField<ColorSpace> {
public:
    InterpolatedLightField(const LightFieldReference<ColorSpace> &source,
                           const Dimension dimension,
                           const visualcloud::interpolator<ColorSpace> interpolator)
            : source_(source), interpolator_(interpolator)
    { }

    inline const typename ColorSpace::Color value(const Point6D &point) const override {
        // TODO this ignores interpolation dimension
        auto value = source_->value(point);
        return value != ColorSpace::Color::Null
            ? value
            : interpolator_(source_, point);
    }

    const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return {source_}; }
    const std::vector<Volume> volumes() const override { return source_->volumes(); }
    const ColorSpace colorSpace() const override { return ColorSpace::Instance; }

private:
    const LightFieldReference<ColorSpace> source_;
    const visualcloud::interpolator<ColorSpace> interpolator_;
};

template<typename Geometry, typename ColorSpace>
class PanoramicLightField: public DiscreteLightField<ColorSpace> {
public:
    PanoramicLightField(const Point3D &point, const TemporalRange &range)
        : DiscreteLightField<ColorSpace>(Geometry::Instance),
          point_(point), volume_{point.ToVolume(range, AngularRange::ThetaMax, AngularRange::PhiMax)}
    { }

    PanoramicLightField(const Point3D &&point, const TemporalRange &&range)
        : DiscreteLightField<ColorSpace>(Geometry::Instance),
          point_(point), volume_{point.ToVolume(range, AngularRange::ThetaMax, AngularRange::PhiMax)}
    { }

    PanoramicLightField(const TemporalRange &range)
        : PanoramicLightField(Point3D{0, 0, 0}, range)
    { }

    PanoramicLightField(const TemporalRange &&range)
        : PanoramicLightField(Point3D{0, 0, 0}, range)
    { }

    virtual ~PanoramicLightField() { }

    const typename ColorSpace::Color value(const Point6D &point) const override {
        return volume_.Contains(point) && defined_at(point)
                ? value(point.t, point.theta, point.phi)
                : ColorSpace::Color::Null;
    }

    const std::vector<Volume> volumes() const override { return {volume_}; }

protected:
    bool defined_at(const Point6D &point) const override {
        return Geometry::Instance.defined_at(point);
    }

    virtual const typename ColorSpace::Color value(const double t, const double theta, const double phi) const {
        return texture(Geometry::Instance.u(theta, phi), Geometry::Instance.v(theta, phi), t);
    }

    virtual const typename ColorSpace::Color texture(const double u, const double v, const double t) const = 0;

private:
    const Point3D point_;
    const Volume volume_;
};

template<typename Geometry, typename ColorSpace>
class PanoramicVideoLightField: public PanoramicLightField<Geometry, ColorSpace> {
public:
    PanoramicVideoLightField(const std::string &filename)  //TODO drop filename; see below
        : PanoramicVideoLightField(Point3D::Zero, std::ifstream{filename}, filename)
    { }

    PanoramicVideoLightField(std::istream &&stream, const std::string &filename="")  //TODO drop filename; see below
        : PanoramicVideoLightField(Point3D::Zero, std::move(stream), filename)
    { }

    PanoramicVideoLightField(const Point3D &point, std::istream &&stream, const std::string &filename="")  //TODO drop filename; see below
          : PanoramicVideoLightField(point, visualcloud::utility::ffmpeg::decode(stream), filename)
    { }

    const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return {}; }
    inline visualcloud::utility::ffmpeg::FrameIterator& frames() const { return *frames_;}
    inline visualcloud::rational framerate() const { return frames_->framerate(); }

    inline std::string filename() const { return filename_; } //TODO drop filename; see below
    inline visualcloud::utility::StreamMetadata metadata() const { return (metadata_.has_value()
                                                                          ? metadata_
                                                                          : (metadata_ = visualcloud::utility::StreamMetadata(filename_, 0, true))).value(); }
protected:
    PanoramicVideoLightField(const Point3D &point,
                             std::unique_ptr<visualcloud::utility::ffmpeg::FrameIterator> frames,
                             const std::string &filename)
            : PanoramicLightField<Geometry, ColorSpace>(point, {0, frames_->duration()}),
              frames_(std::move(frames)),
              geometry_(Dimension::Time, frames_->framerate()),
              filename_(filename)
    { }

    bool defined_at(const Point6D &point) const override {
        return geometry_.defined_at(point) &&
               PanoramicLightField<Geometry, ColorSpace>::defined_at(point);
    }

    const typename ColorSpace::Color texture(const double u, const double v, const double t) const override {
        return YUVColorSpace::Color::Red; //TODO
        //throw std::runtime_error("TODO"); //TODO
    };

private:
    const std::string filename_; //TODO drop filename after adding StreamDecodeReader in Physical.cc
    const std::unique_ptr<visualcloud::utility::ffmpeg::FrameIterator> frames_;
    const IntervalGeometry geometry_;
    mutable std::optional<visualcloud::utility::StreamMetadata> metadata_;
};

#endif //VISUALCLOUD_LIGHTFIELD_H
