#ifndef VISUALCLOUD_GEOMETRY_H
#define VISUALCLOUD_GEOMETRY_H

#include "rational.h"
#include <stdexcept>
#include <cmath>

enum Dimension {
    X,
    Y,
    Z,
    Time,
    Theta,
    Phi
};

struct SpatiotemporalRange {
    double start;
    double end;

    bool Contains(double value) const {
        return start <= value && value <= end;
    }

    static const SpatiotemporalRange SpatialMax;
    static const SpatiotemporalRange TemporalMax;
};

typedef struct SpatiotemporalRange SpatialRange;
typedef struct SpatiotemporalRange TemporalRange;

struct AngularRange {
public:
    double start;
    double end;

    bool Contains(const double angle) const {
        return start <= angle && angle <= end;
    }

    static const AngularRange ThetaMax;
    static const AngularRange PhiMax;
};

struct Point6D;

class Volume {
public:
    SpatialRange x;
    SpatialRange y;
    SpatialRange z;
    TemporalRange t;
    AngularRange theta;
    AngularRange phi;

    static const Volume VolumeMax;

    bool Contains(const Point6D &point) const;
};

struct Point3D {
public:
    double x;
    double y;
    double z;

    inline operator Volume() const {
        return Volume{{x, x}, {y, y}, {z, z}, TemporalRange::TemporalMax, AngularRange::ThetaMax, AngularRange::PhiMax};
    }

    inline Volume ToVolume(TemporalRange timeRange, AngularRange thetaRange=AngularRange::ThetaMax, AngularRange phiRange=AngularRange::PhiMax) const {
        return Volume{{x, x}, {y, y}, {z, z}, timeRange, thetaRange, phiRange};
    }

    inline double get(Dimension dimension) const {
        switch(dimension) {
            case Dimension::X:
                return x;
            case Dimension::Y:
                return y;
            case Dimension::Z:
                return z;
            default:
                throw std::invalid_argument("Invalid dimension"); //TODO
        }
    }

    static const Point3D Zero;
};

struct Point4D: Point3D {
public:
    double t;

    operator Volume() {
        return Volume{{x, x}, {y, y}, {z, z}, {t, t}, AngularRange::ThetaMax, AngularRange::PhiMax};
    }

    inline double get(Dimension dimension) const {
        switch(dimension) {
            case Dimension::Time:
                return t;
            default:
                return Point3D::get(dimension);
        }
    }

    static const Point4D Zero;
};

struct Point6D: Point4D {
public:
    double theta;
    double phi;

    operator Volume() {
        return Volume{{x, x}, {y, y}, {z, z}, {t, t}, {theta, theta}, {phi, phi}};
    }

    inline double get(Dimension dimension) const {
        switch(dimension) {
            case Dimension::Theta:
                return theta;
            case Dimension::Phi:
                return phi;
            default:
                return Point4D::get(dimension);
        }
    }

    static const Point6D Zero;
};

class Geometry {
public:
    virtual bool defined_at(const Point3D &point) const = 0;
};

class MeshGeometry: public Geometry {
public:
    virtual double u(const double theta, const double phi) const = 0;
    virtual double v(const double theta, const double phi) const = 0;

    bool defined_at(const Point3D &point) const override {
        return true; //TODO
    }
};

class EquirectangularGeometry: public MeshGeometry {
public:
    double u(const double theta, const double phi) const override { return 0; }
    double v(const double theta, const double phi) const override { return 0; }

    bool defined_at(const Point3D &point) const override {
        return true; //TODO
    }

    static const EquirectangularGeometry Instance;
};

class IntervalGeometry: public Geometry {
public:
    IntervalGeometry(const Dimension dimension, const visualcloud::rational interval)
        : dimension_(dimension), interval_(interval)
    { }

    bool defined_at(const Point3D &point) const override {
        //TODO holy cow this is like the worst thing ever
        //TODO add rational overloads for value() and Volume::contains()
        return std::remainder(
                point.get(dimension()),
                this->interval().numerator() / (double)this->interval().denominator()) <= 0.000001l;
    }

    inline const Dimension dimension() const { return dimension_; }
    inline const visualcloud::rational interval() const { return interval_; }

private:
    const Dimension dimension_;
    const visualcloud::rational interval_;
};


#endif //VISUALCLOUD_GEOMETRY_H
