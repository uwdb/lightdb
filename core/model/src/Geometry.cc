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

    std::vector<Volume> Volume::partition(const Dimension dimension, const lightdb::rational &interval) const {
        std::vector<Volume> result; //TODO should return an iterator, not a materialized vector
        double span = (double)interval.numerator() / interval.denominator(); //TODO horrible!
        double current;

        asserts::CHECK_POSITIVE(span);

        //TODO add a base class for all dimensions and drop switch
        switch(dimension) {
            case Dimension::X:
                assert(x() != limits().x());
                for(current = x().start(); current < x().end(); current += span)
                    result.push_back(Volume{{current, std::min(x().end(), current + span)}, y(), z(), t(), theta(), phi()});
                break;
            case Dimension::Y:
                assert(y() != limits().y());
                for(current = y().start(); current < y().end(); current += span)
                    result.push_back(Volume{x(), {current, std::min(y().end(), current + span)}, z(), t(), theta(), phi()});
                break;
            case Dimension::Z:
                assert(z() != limits().z());
                for(current = z().start(); current < z().end(); current += span)
                    result.push_back(Volume{x(), y(), {current, std::min(z().end(), current + span)}, t(), theta(), phi()});
                break;
            case Dimension::Time:
                assert(t() != limits().t());
                for(current = t().start(); current < t().end(); current += span)
                    result.push_back(Volume{x(), y(), z(), {current, std::min(t().end(), current + span)}, theta(), phi()});
                break;
            case Dimension::Theta:
                for(current = theta().start(); current < theta().end(); current += span)
                    result.push_back(Volume{x(), y(), z(), t(), {current, std::min(theta().end(), current + span)}, phi()});
                break;
            case Dimension::Phi:
                for(current = phi().start(); current < phi().end(); current += span)
                    result.push_back(Volume{x(), y(), z(), t(), theta(), {current, std::min(phi().end(), current + span)}});
                break;
        }

        return result;
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