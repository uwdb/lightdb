#ifndef VISUALCLOUD_LIGHTFIELD_H
#define VISUALCLOUD_LIGHTFIELD_H

#include "Geometry.h"
#include "Color.h"
#include <vector>
#include <stdexcept>
#include <math.h>

template<typename ColorSpace>
class LightField { //: public LightField {
protected:
    LightField() { }

public:
    virtual const typename ColorSpace::Color value(const double x, const double y, const double z, const double t, const double theta, const double phi) const = 0;
    virtual const ColorSpace colorSpace() const { return ColorSpace::Instance; }
    virtual const std::vector<Volume> volumes() const = 0;
};

template<typename ColorSpace>
class ConstantLightField: public LightField<ColorSpace> {
public:
    ConstantLightField(const typename ColorSpace::Color& color)
            : colorSpace_(ColorSpace::Instance), color_(color)
    { }

    const typename ColorSpace::Color value(const double x, const double y, const double z, const double t, const double theta, const double phi) const override {
        return color_;
    }

    const std::vector<Volume> volumes() const override { return {Volume::VolumeMax}; }
    const ColorSpace colorSpace() const override { return colorSpace_; }

private:
    const ColorSpace colorSpace_;
    const typename ColorSpace::Color color_;
};

template<typename Geometry, typename ColorSpace>
class DiscreteLightField: public LightField<ColorSpace> {
protected:
    DiscreteLightField() { }
};

template<typename Geometry, typename ColorSpace>
class PanoramicLightField: public DiscreteLightField<Geometry, ColorSpace> {
public:
    PanoramicLightField(const Point3D &point, const TemporalRange &range)
            : point_(point), volume_{point.ToVolume(range, AngularRange::ThetaMax, AngularRange::PhiMax)}
    { }

    PanoramicLightField(const Point3D &&point, const TemporalRange &&range)
            : point_(point), volume_{point.ToVolume(range, AngularRange::ThetaMax, AngularRange::PhiMax)}
    { }

    PanoramicLightField(const TemporalRange &range)
            : PanoramicLightField(Point3D{0, 0, 0}, range)
        { }

    PanoramicLightField(const TemporalRange &&range)
            : PanoramicLightField(Point3D{0, 0, 0}, range)
        { }

    const typename ColorSpace::Color value(const double x, const double y, const double z, const double t, const double theta, const double phi) const override {
        return volume_.Contains(x, y, z, t, theta, phi)
                ? value(t, theta, phi)
                : ColorSpace::Color::Null;
    }

    const std::vector<Volume> volumes() const override { return {volume_}; }

protected:
    virtual const typename ColorSpace::Color value(const double t, const double theta, const double phi) const { // = 0;
        auto u = Geometry::Instance.u(theta, phi),
             v = Geometry::Instance.v(theta, phi);

        throw std::runtime_error("TODO2");
        //return ColorSpace::Color::Null;
    }

private:
    const Volume volume_;
    const Point3D point_;
};

#endif //VISUALCLOUD_LIGHTFIELD_H
