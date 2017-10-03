#ifndef VISUALCLOUD_GEOMETRY_H
#define VISUALCLOUD_GEOMETRY_H

struct SpatiotemporalRange {
    double start;
    double end;

    bool Contains(double value) const {
        return start <= value && value <= end;
    }
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

class Volume {
public:
    SpatialRange x;
    SpatialRange y;
    SpatialRange z;
    TemporalRange t;
    AngularRange theta;
    AngularRange phi;

    static const Volume VolumeMax;

    bool Contains(const double x, const double y, const double z, const double t, const double theta, const double phi) const {
        return this->x.Contains(x) && this->y.Contains(y) && this->z.Contains(z) &&
                this->t.Contains(t) &&
                this->theta.Contains(theta) && this->phi.Contains(phi);
    }
};

struct Point3D {
public:
    double x;
    double y;
    double z;

    Volume ToVolume(TemporalRange timeRange, AngularRange thetaRange=AngularRange::ThetaMax, AngularRange phiRange=AngularRange::PhiMax) const {
        return Volume{{x, x}, {y, y}, {z, z}, timeRange, thetaRange, phiRange};
    }
};

struct Point4D {
public:
    double x;
    double y;
    double z;
    double t;
};

class Geometry {

};

class MeshGeometry: public Geometry {
public:
    virtual double u(const double theta, const double phi) const = 0;
    virtual double v(const double theta, const double phi) const = 0;
};

class EquirectangularGeometry: public MeshGeometry {
public:
    double u(const double theta, const double phi) const override { return 0; }
    double v(const double theta, const double phi) const override { return 0; }

    static const EquirectangularGeometry Instance;
};

#endif //VISUALCLOUD_GEOMETRY_H
