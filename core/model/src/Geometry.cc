#include "Geometry.h"
#include <c++/7/limits>
#include <math.h>

const AngularRange AngularRange::ThetaMax{0, M_2_PI};
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
