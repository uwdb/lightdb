#ifndef LIGHTDB_GEOMETRY_H
#define LIGHTDB_GEOMETRY_H

#include "number.h"
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

    namespace Dimensions {
        static const Dimension All[] = {Dimension::X, Dimension::Y, Dimension::Z, Dimension::Time,
                                        Dimension::Theta, Dimension::Phi};
    };

    typedef number angle; //TODO change to union double/rational/inf-precision type, also add to Spatiotemporal range

    class UnknownRange;

    namespace internal {
        namespace limits {
            class spatial {
            public:
                static constexpr const double minimum{-std::numeric_limits<double>::max()};
                static constexpr const double maximum{std::numeric_limits<double>::max()};
            };
            class theta {
            public:
                static constexpr const angle minimum{0};
                static constexpr const angle maximum{2*lightdb::PI};
            };
            class phi {
            public:
                static constexpr const angle minimum{0};
                static constexpr const angle maximum{lightdb::PI};
            };
            using none = spatial;
        }

        template<typename TDerived, typename Value, typename Limits>
        class Range {
        public:
            constexpr Range(const Value start, const Value end)
                    : start_(start), end_(end)  {
                assert(end >= start);
                assert(start >= Limits::minimum);
                assert(end <= Limits::maximum);
            }
            Range(const UnknownRange &range);

            inline constexpr number start() const { return start_; }
            inline constexpr number end() const { return end_; }

            inline number magnitude() const { return end() - start(); }

            inline bool contains(number value) const { return start() <= value && value <= end(); }

            bool empty() const { return start() == end(); }

            static constexpr const TDerived limits() { return {Limits::minimum, Limits::maximum}; }

            operator UnknownRange() const;

            inline TDerived operator|(const TDerived &other) const {
                auto min_start = std::min(start(), other.start());
                return {min_start, std::max(min_start, std::max(end(), other.end()))};
            }

            inline TDerived operator&(const TDerived &other) const {
                auto max_start = std::max(start(), other.start());
                return {max_start, std::max(max_start, std::min(end(), other.end()))};
            }

            bool operator==(const TDerived &other) const {
                return this == &other ||
                        (start().epsilon_equal(other.start()) && end().epsilon_equal(other.end()));
            }

            bool operator!=(const TDerived &other) const {
                return !operator==(other);
            }

        private:
            Value start_;
            Value end_;
        };
    }

    class UnknownRange: public internal::Range<UnknownRange, number, internal::limits::none>     {
        using internal::Range<UnknownRange, number, internal::limits::none>::Range;
    };
    class SpatiotemporalRange: public internal::Range<SpatiotemporalRange, number, internal::limits::spatial>     {
        using internal::Range<SpatiotemporalRange, number, internal::limits::spatial>::Range;
    };
    class ThetaRange: public internal::Range<ThetaRange, angle, internal::limits::theta> {
        using internal::Range<ThetaRange, angle, internal::limits::theta>::Range;
    };
    class PhiRange: public internal::Range<PhiRange, angle, internal::limits::phi> {
        using internal::Range<PhiRange, angle, internal::limits::phi>::Range;
    };

    using SpatialRange = SpatiotemporalRange;
    using TemporalRange = SpatiotemporalRange;

    template<typename TDerived, typename Value, typename Limits>
    internal::Range<TDerived, Value, Limits>::operator UnknownRange() const {
        return {start(), end()};
    };
    template<typename TDerived, typename Value, typename Limits>
    internal::Range<TDerived, Value, Limits>::Range(const UnknownRange &range)
            : Range(range.start(), range.end())
    { }


    class Point3D;
    class Point4D;
    class Point6D;

    class Volume {
    public:
        class iterable;
        class iterator;

        constexpr Volume(const SpatialRange &x, const SpatialRange &y, const SpatialRange &z)
            : Volume(x, y, z, TemporalRange::limits())
        { }

        constexpr Volume(const SpatialRange &x, const SpatialRange &y, const SpatialRange &z, const TemporalRange &t)
            : Volume(x, y, z, t, ThetaRange::limits(), PhiRange::limits())
        { }

        constexpr Volume(SpatialRange x, SpatialRange y, SpatialRange z, TemporalRange t,
                         ThetaRange theta, PhiRange phi)
            : x_(std::move(x)), y_(std::move(y)), z_(std::move(z)), t_(std::move(t)),
              theta_(std::move(theta)), phi_(std::move(phi))
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

        UnknownRange get(Dimension dimension) const;
        Volume& set(SpatiotemporalDimension dimension, const SpatiotemporalRange &range);
        Volume& set(Dimension dimension, const UnknownRange &range);

        bool contains(const Point6D &point) const;
        bool has_nonempty_intersection(const Volume &other) const;
        iterable partition(Dimension, const number&) const;
        Volume translate(const Point6D&) const;

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

        inline Volume operator&(const Volume &other) const {
            return {x() & other.x(), y() &other.y(), z() & other.z(), t() & other.t(),
                    theta() & other.theta(), phi() & other.phi()};
        }

        bool operator==(const Volume &other) const {
            return this == &other ||
                   (x() == other.x() && y() == other.y() && z() == other.z() && t() == other.t() &&
                    theta() == other.theta() && phi() == other.phi());
        }

        bool operator!=(const Volume &other) const {
            return !operator==(other);
        }

        static constexpr const Volume limits() { return {
                    SpatialRange::limits(), SpatialRange::limits(), SpatialRange::limits(),
                    TemporalRange::limits(),
                    ThetaRange::limits(), PhiRange::limits()}; }

    private:
        SpatialRange x_;
        SpatialRange y_;
        SpatialRange z_;
        TemporalRange t_;
        ThetaRange theta_;
        PhiRange phi_;
    };

    class Volume::iterator: public std::iterator<std::input_iterator_tag, Volume, void, Volume*, Volume&> {
    public:
        explicit iterator(Volume volume, const Dimension dimension, const number &interval)
            : volume_(std::move(volume)), dimension_(dimension), interval_(interval)
        { }

        iterator& operator++() {
            volume_.set(dimension_,
                        {std::min(volume_.get(dimension_).start() + interval_, volume_.get(dimension_).end()),
                         volume_.get(dimension_).end()});
            return *this;
        }

        iterator operator++(int) {
            auto result = *this;
            ++(*this);
            return result;
        }

        bool operator==(const iterator other) const {
            return volume_.get(dimension_).start() >= other.volume_.get(dimension_).start(); }
        bool operator!=(const iterator &other) const { return !(*this == other); }

        Volume operator*() const {
            return Volume(volume_).set(dimension_, {volume_.get(dimension_).start(),
                                                    std::min(volume_.get(dimension_).start() + interval_,
                                                             volume_.get(dimension_).end())});
        }
    private:
        Volume volume_;
        const Dimension dimension_;
        number interval_;
    };

    class Volume::iterable {
    public:
        iterable(Volume model, const Dimension dimension, const number &interval)
                : model_(std::move(model)), dimension_(dimension), interval_(interval)
        { }

        iterator begin() const;
        iterator end() const;
        explicit operator std::vector<Volume>() { return std::vector<Volume>{begin(), end()}; }

    private:
        const Volume model_;
        const Dimension dimension_;
        const number interval_;
    };

    class CompositeVolume {
    public:
        CompositeVolume(const Volume &volume)
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
        constexpr Point3D(const number &x, const number &y, const number &z)
            : x_(x), y_(y), z_(z)
        { }

        inline constexpr number x() const { return x_; }
        inline constexpr number y() const { return y_; }
        inline constexpr number z() const { return z_; }

        virtual inline number get(Dimension dimension) const {
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
        number x_, y_, z_;
    };

    class Point4D : public Point3D {
    public:
        constexpr Point4D(const number &x, const number &y, const number &z, const number &t)
            : Point3D(x, y, z), t_(t)
        { }

        inline constexpr number t() const { return t_; }

        inline number get(Dimension dimension) const override {
            switch (dimension) {
                case Dimension::Time:
                    return t();
                default:
                    return Point3D::get(dimension);
            }
        }

        static constexpr const Point4D zero() { return {0, 0, 0, 0}; };

    private:
        number t_;
    };

    class Point6D : public Point4D {
    public:
        constexpr Point6D(const number &x, const number &y, const number &z, const number &t,
                          const angle &theta, const angle &phi)
            : Point4D(x, y, z, t), theta_(theta), phi_(phi)
        { }

        inline constexpr angle theta() const { return theta_; }
        inline constexpr angle phi() const { return phi_; }

        Point6D operator+(const Point6D &other) const {
            return {x() + other.x(), y() + other.y(), z() + other.z(),
                    t() + other.t(),
                    theta() + other.theta(), phi() + other.phi()};
        }

        inline number get(Dimension dimension) const override {
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
        number theta_, phi_;
    };

    class Geometry {
    public:
        virtual bool is_monotonic() const = 0;
        virtual bool defined_at(const Point6D &point) const = 0;
    };

    using GeometryReference = shared_reference<Geometry>;

    class MeshGeometry : public Geometry {
    public:
        virtual double u(angle theta, angle phi) const = 0;

        virtual double v(angle theta, angle phi) const = 0;

        bool is_monotonic() const override { return false; }

        bool defined_at(const Point6D &point) const override {
            return true; //TODO
        }
    };

    class EquirectangularGeometry : public MeshGeometry {
    public:
        class Samples
        {
            size_t theta;
            size_t phi;
        };

        explicit EquirectangularGeometry(Samples samples)
                : samples_(samples)
        { }


        double u(const angle theta, const angle phi) const override { return 0; }

        double v(const angle theta, const angle phi) const override { return 0; }

        bool is_monotonic() const override { return true; }

        bool defined_at(const Point6D &point) const override {
            return true; //TODO
        }

        private:
            const Samples samples_;

//        static constexpr const EquirectangularGeometry instance() { return EquirectangularGeometry(); }
    };

    class IntervalGeometry : public Geometry {
    public:
        constexpr IntervalGeometry(const Dimension dimension, const number &interval)
                : dimension_(dimension), interval_(interval) { }

        template<typename tolerance>
        bool defined_at(const Point6D &point) const {
            return point.get(dimension()).epsilon_equal<tolerance>(interval());
        }

        bool is_monotonic() const override { return true; }

        bool defined_at(const Point6D &point) const override {
            return this->defined_at<std::pico>(point);
        }

        inline constexpr const Dimension dimension() const { return dimension_; }
        inline constexpr const number interval() const { return interval_; }
        inline constexpr std::optional<unsigned long> size() const {
            if(dimension() == Dimension::Theta)
                return {static_cast<unsigned long>(2 * PI / interval())};
            else if(dimension() == Dimension::Phi)
                return {static_cast<unsigned long>(PI / interval())};
            else
                return {};
        }

    private:
        const Dimension dimension_;
        const number interval_;
    };

} // namespace lightdb

#endif //LIGHTDB_GEOMETRY_H
