#include "Geometry.h"

namespace lightdb
{
    Volume::Volume(const Point3D& point, const TemporalRange& t, const ThetaRange& theta, const PhiRange& phi)
        : x_{point.x(), point.x()}, y_{point.y(), point.y()}, z_{point.z(), point.z()}, t_(t),
          theta_(theta), phi_(phi)
    { }

    Volume::Volume(const Point4D& point, const ThetaRange& theta, const PhiRange& phi)
        : Volume(point, {point.t(), point.t()}, theta, phi)
    { }

    Volume::Volume(const Point6D& point)
        : Volume(point, {point.theta(), point.theta()}, {point.phi(), point.phi()})
    { }

    bool Volume::contains(const Point6D &point) const {
        return x().contains(point.x()) && y().contains(point.y()) && z().contains(point.z()) &&
               t().contains(point.t()) &&
               theta().contains(point.theta()) && phi().contains(point.phi());
    }

    Volume& Volume::set(const SpatiotemporalDimension dimension, const SpatiotemporalRange &range) {
        switch(dimension) {
            case SpatiotemporalDimension::X:
                x_ = range;
                break;
            case SpatiotemporalDimension::Y:
                y_ = range;
                break;
            case SpatiotemporalDimension::Z:
                z_ = range;
                break;
            case SpatiotemporalDimension::Time:
                t_ = range;
                break;
        }
        return *this;
    }
    Volume& Volume::set(const Dimension dimension, const UnknownRange &range) {
        switch(dimension) {
            case Dimension::X:
            case Dimension::Y:
            case Dimension::Z:
            case Dimension::Time:
                Volume::set((SpatiotemporalDimension)dimension, range);
                break;
            case Dimension::Theta:
                theta(range);
            case Dimension::Phi:
                phi(range);
        }
        return *this;
    };
    UnknownRange Volume::get(const Dimension dimension) const {
        switch(dimension) {
            case Dimension::X:
                return x();
            case Dimension::Y:
                return y();
            case Dimension::Z:
                return z();
            case Dimension::Time:
                return t();
            case Dimension::Theta:
                return theta();
            case Dimension::Phi:
                return phi();
        }
        throw InvalidArgumentError("Invalid dimension", "dimension");
    };

    Volume::iterator Volume::iterable::begin() const { return iterator(model_, dimension_, interval_); }
    Volume::iterator Volume::iterable::end() const {
        return iterator(Volume{model_}.set(dimension_, {model_.get(dimension_).end(),
                                                        model_.get(dimension_).end()}), dimension_, interval_); }
    Volume::iterable Volume::partition(const Dimension dimension, const number& interval) const {
        return Volume::iterable(*this, dimension, interval);
    }

    Volume Volume::translate(const Point6D &delta) const{
        return {{x().start() + delta.x(),         x().end() + delta.x()},
                {y().start() + delta.y(),         y().end() + delta.y()},
                {z().start() + delta.z(),         z().end() + delta.z()},
                {t().start() + delta.t(),         t().end() + delta.t()},
                {theta().start() + delta.theta(), theta().end() + delta.theta()},
                {phi().start() + delta.phi(),     phi().end() + delta.phi()}};
    }

} // namespace lightdb