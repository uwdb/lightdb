#ifndef LIGHTDB_GEOMETRY_H
#define LIGHTDB_GEOMETRY_H

#include "rational.h"
#include "asserts.h"
#include <utility>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <numeric>
#include <reference.h>

namespace lightdb {

    enum class Dimension {
        X,
        Y,
        Z,
        Time,
        Theta,
        Phi
    };

    enum class SpatiotemporalDimension {
        X,
        Y,
        Z,
        Time,
    };

    enum class AngularDimension {
        Theta,
        Phi
    };

    namespace internal {
        template<typename TDerived, typename Value>
        class Range {
        public:
            inline constexpr double start() const { return start_; }
            inline constexpr double end() const { return end_; }

            inline double magnitude() const { return end() - start(); }

            inline bool contains(double value) const { return start() <= value && value <= end(); }

            bool empty() const { return start() == end(); }

            inline TDerived operator|(const TDerived &other) const {
                auto max_start = std::max(start(), other.start());
                return {max_start, std::max(max_start, std::min(end(), other.end()))};
            }

            bool operator==(const TDerived &other) const {
                static double epsilon = 0.000001; //TODO ugh
                return this == &other ||
                       (std::abs(start() - other.start()) < epsilon && std::abs(end() - other.end()) < epsilon);
            }

            bool operator!=(const TDerived &other) const {
                return !operator==(other);
            }

        protected:
            constexpr Range(const Value start, const Value end)
                    : start_(start), end_(end)  {
                assert(end >= start);
                assert(start >= TDerived::limits().start());
                assert(end <= TDerived::limits().end());
            }

        private:
            Value start_;
            Value end_;
        };
    }

    class SpatiotemporalRange: public internal::Range<SpatiotemporalRange, double>
    {
    public:
        constexpr SpatiotemporalRange(const double start, const double end) : Range(start, end)  { }
        static constexpr const SpatiotemporalRange limits() { return {-std::numeric_limits<double>::max(),
                                                                      std::numeric_limits<double>::max()}; }
    };

    using SpatialRange = SpatiotemporalRange;
    using TemporalRange = SpatiotemporalRange;
    typedef double angle; //TODO change to union double/rational/inf-precision type, also add to Spatiotemporal range

    class ThetaRange: public internal::Range<ThetaRange, angle> {
    public:
        constexpr ThetaRange(const angle start, const angle end) : Range(start, end)  { }
        static constexpr const ThetaRange limits() { return {0, 2*M_PI}; }
    };
    class PhiRange: public internal::Range<PhiRange, angle> {
    public:
        constexpr PhiRange(const angle start, const angle end) : Range(start, end)  { }
        static constexpr const PhiRange limits() { return {0, M_PI}; }
    };

    class Point3D;
    class Point4D;
    class Point6D;

    class Volume {
    public:
        constexpr Volume(const SpatialRange &x, const SpatialRange &y, const SpatialRange &z)
            : Volume(x, y, z, TemporalRange::limits())
        { }

        constexpr Volume(const SpatialRange &x, const SpatialRange &y, const SpatialRange &z, const TemporalRange &t)
            : Volume(x, y, z, t, ThetaRange::limits(), PhiRange::limits())
        { }

        constexpr Volume(const SpatialRange x, const SpatialRange y, const SpatialRange z, const TemporalRange t,
                         const ThetaRange theta, const PhiRange phi)
            : x_(x), y_(y), z_(z), t_(t), theta_(theta), phi_(phi)
        { }

        Volume(const Point3D&, const TemporalRange&, const ThetaRange&, const PhiRange&);
        Volume(const Point4D&, const ThetaRange&, const PhiRange&);
        Volume(const Point6D&);

        inline const SpatialRange& x() const { return x_; }
        inline const SpatialRange& y() const { return y_; }
        inline const SpatialRange& z() const { return z_; }
        inline const TemporalRange& t() const { return t_; }
        inline const ThetaRange& theta() const { return theta_; }
        inline const PhiRange& phi() const { return phi_; }

        inline const SpatialRange& x(const SpatialRange& value) { return x_ = value; }
        inline const SpatialRange& y(const SpatialRange& value) { return y_ = value; }
        inline const SpatialRange& z(const SpatialRange& value) { return z_ = value; }
        inline const TemporalRange& t(const TemporalRange& value) { return t_ = value; }
        inline const ThetaRange& theta(const ThetaRange& value) { return theta_ = value; }
        inline const PhiRange& phi(const PhiRange& value) { return phi_ = value; }

        static constexpr const Volume limits() { return {
                    SpatialRange::limits(), SpatialRange::limits(), SpatialRange::limits(),
                    TemporalRange::limits(),
                    ThetaRange::limits(), PhiRange::limits()}; }

        bool contains(const Point6D &point) const;
        std::vector<Volume> partition(Dimension, const rational&) const;
        Volume translate(const Point6D&) const;
        inline const SpatiotemporalRange& set(const SpatiotemporalDimension dimension, const SpatiotemporalRange &range)
        {
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
            return range;
        }

        inline bool is_point() const {
            return x().start() == x().end() &&
                   y().start() == y().end() &&
                   z().start() == z().end() &&
                   t().start() == t().end() &&
                   theta().start() == theta().end() &&
                   phi().start() == phi().end();
        }

        inline Volume operator|(const Volume &other) const {
            return {x() | other.x(), y() | other.y(), z() | other.z(), t() | other.t(),
                    theta() | other.theta(), phi() | other.phi()};
        }

        bool operator==(const Volume &other) const {
            return this == &other ||
                   (x() == other.x() && y() == other.y() && z() == other.z() && t() == other.t() &&
                    theta() == other.theta() && phi() == other.phi());
        }

        bool operator!=(const Volume &other) const {
            return !operator==(other);
        }

    private:
        SpatialRange x_;
        SpatialRange y_;
        SpatialRange z_;
        TemporalRange t_;
        ThetaRange theta_;
        PhiRange phi_;
    };

    class CompositeVolume {
    public:
        CompositeVolume(const Volume volume)
                : components_({volume}), bounding_(volume) {}

        explicit CompositeVolume(const std::vector<Volume> &volumes)
                : components_(asserts::CHECK_NONEMPTY(volumes)),
                  bounding_(std::accumulate(volumes.begin() + 1, volumes.end(), volumes[0],
                                           [](auto &result, auto &current) { return current | result; }))
        { }

        explicit operator const Volume &() const {
            return bounding_;
        }

        const Volume bounding() const { return bounding_; }
        const std::vector<Volume>& components() const { return components_; }

    private:
        const std::vector<Volume> components_;
        const Volume bounding_;
    };

    class Point3D {
    public:
        constexpr Point3D(const double x, const double y, const double z)
            : x_(x), y_(y), z_(z)
        { }

        inline constexpr double x() const { return x_; }
        inline constexpr double y() const { return y_; }
        inline constexpr double z() const { return z_; }

        virtual inline double get(Dimension dimension) const {
            switch (dimension) {
                case Dimension::X:
                    return x();
                case Dimension::Y:
                    return y();
                case Dimension::Z:
                    return z();
                default:
                    throw InvalidArgumentError("Invalid dimension", "dimension");
            }
        }

        static constexpr const Point3D zero() { return {0, 0, 0}; };

    private:
        double x_, y_, z_;
    };

    class Point4D : public Point3D {
    public:
        constexpr Point4D(const double x, const double y, const double z, const double t)
            : Point3D(x, y, z), t_(t)
        { }

        inline constexpr double t() const { return t_; }

        inline double get(Dimension dimension) const override {
            switch (dimension) {
                case Dimension::Time:
                    return t();
                default:
                    return Point3D::get(dimension);
            }
        }

        static constexpr const Point4D zero() { return {0, 0, 0, 0}; };

    private:
        double t_;
    };

    class Point6D : public Point4D {
    public:
        constexpr Point6D(const double x, const double y, const double z, const double t,
                          const angle theta, const angle phi)
            : Point4D(x, y, z, t), theta_(theta), phi_(phi)
        { }

        inline constexpr angle theta() const { return theta_; }
        inline constexpr angle phi() const { return phi_; }

        Point6D operator+(const Point6D &other) const {
            return {x() + other.x(), y() + other.y(), z() + other.z(),
                    t() + other.t(),
                    theta() + other.theta(), phi() + other.phi()};
        }

        inline double get(Dimension dimension) const override {
            switch (dimension) {
                case Dimension::Theta:
                    return theta();
                case Dimension::Phi:
                    return phi();
                default:
                    return Point4D::get(dimension);
            }
        }

        static constexpr const Point6D zero() { return {0, 0, 0, 0, 0, 0}; }

    private:
        double theta_, phi_;
    };

    class Geometry {
    public:
        virtual bool defined_at(const Point6D &point) const = 0;
    };

    using GeometryReference = shared_reference<Geometry>;

    class MeshGeometry : public Geometry {
    public:
        virtual double u(angle theta, angle phi) const = 0;

        virtual double v(angle theta, angle phi) const = 0;

        bool defined_at(const Point6D &point) const override {
            return true; //TODO
        }
    };

    class EquirectangularGeometry : public MeshGeometry {
    public:
        double u(const angle theta, const angle phi) const override { return 0; }

        double v(const angle theta, const angle phi) const override { return 0; }

        bool defined_at(const Point6D &point) const override {
            return true; //TODO
        }

        static constexpr const EquirectangularGeometry instance() { return EquirectangularGeometry(); }
    };

    class IntervalGeometry : public Geometry {
    public:
        constexpr IntervalGeometry(const Dimension dimension, const lightdb::rational &interval)
                : dimension_(dimension), interval_(interval) {}

        bool defined_at(const Point6D &point) const override {
            //TODO holy cow this is like the worst thing ever
            //TODO add rational overloads for value() and Volume::contains()
            return std::remainder(
                    point.get(dimension()),
                    interval().numerator() / (double)interval().denominator()) <= 0.000001l;
        }

        inline constexpr const Dimension dimension() const { return dimension_; }
        inline constexpr const lightdb::rational interval() const { return interval_; }

    private:
        const Dimension dimension_;
        const lightdb::rational interval_;
    };

} // namespace lightdb

#endif //LIGHTDB_GEOMETRY_H
