#include "Geometry.h"
#include <c++/7/limits>
#include <math.h>

const AngularRange AngularRange::ThetaMax{0, 2*M_PI};
const AngularRange AngularRange::PhiMax{0, M_PI};
const SpatialRange SpatialRange::SpatialMax{-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
const TemporalRange TemporalRange::TemporalMax{-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
const Volume Volume::VolumeMax{SpatialRange::SpatialMax, SpatialRange::SpatialMax, SpatialRange::SpatialMax,
                               TemporalRange::TemporalMax,
                               AngularRange::ThetaMax, AngularRange::PhiMax};

const Point3D Point3D::Zero{0, 0, 0};
const Point4D Point4D::Zero{0, 0, 0, 0};
const Point6D Point6D::Zero{0, 0, 0, 0, 0, 0};

const EquirectangularGeometry EquirectangularGeometry::Instance;


bool Volume::Contains(const Point6D &point) const {
    return this->x.Contains(point.x) && this->y.Contains(point.y) && this->z.Contains(point.z) &&
           this->t.Contains(point.t) &&
           this->theta.Contains(point.theta) && this->phi.Contains(point.phi);
}

std::vector<Volume> Volume::partition(const Dimension dimension, const lightdb::rational &interval) const {
    std::vector<Volume> result;
    double span = (double)interval.numerator() / interval.denominator(); //TODO horrible!
    double current;

    assert(span > 0);

    //TODO add a base class for all dimensions and drop switch
    switch(dimension) {
        case X:
            for(current = x.start; current < x.end; current += span)
                result.push_back(Volume{{current, std::min(x.end, current + span)}, y, z, t, theta, phi});
            break;
        case Y:
            for(current = y.start; current < y.end; current += span)
                result.push_back(Volume{x, {current, std::min(y.end, current + span)}, z, t, theta, phi});
            break;
        case Z:
            for(current = z.start; current < z.end; current += span)
                result.push_back(Volume{x, y, {current, std::min(z.end, current + span)}, t, theta, phi});
            break;
        case Time:
            for(current = t.start; current < t.end; current += span)
                result.push_back(Volume{x, y, z, {current, std::min(t.end, current + span)}, theta, phi});
            break;
        case Theta:
            for(current = theta.start; current < theta.end; current += span)
                result.push_back(Volume{x, y, z, t, {current, std::min(theta.end, current + span)}, phi});
            break;
        case Phi:
            for(current = phi.start; current < phi.end; current += span)
                result.push_back(Volume{x, y, z, t, theta, {current, std::min(phi.end, current + span)}});
            break;
    }

    return result;
}

Volume Volume::translate(const Point6D &delta) const{
    return {{x.start + delta.x,         x.end + delta.x},
            {y.start + delta.y,         y.end + delta.y},
            {z.start + delta.z,         z.end + delta.z},
            {t.start + delta.t,         t.end + delta.t},
            {theta.start + delta.theta, theta.end + delta.theta},
            {phi.start + delta.phi,     phi.end + delta.phi}};
}
