#ifndef LIGHTDB_NUMBER_H
#define LIGHTDB_NUMBER_H

#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/rational.hpp>
#include <variant>
#include <ratio>

namespace lightdb {
    static constexpr const long double PI = 3.141592653589793238512808959406186204433;
    static constexpr const long double TWOPI = 2*PI;

    #define _LOGICAL_RATIONAL_OPERATOR(op)                   \
    bool operator op(const rational& other) const noexcept { \
        return static_cast<rational_base>(*this) op static_cast<rational_base>(other); }

    #define _ARITHMATIC_RATIONAL_OPERATOR(op)                  \
    inline rational operator op(const rational& other) const { \
        return rational{static_cast<rational_base>(*this) op static_cast<rational_base>(other)}; }


    #define _LOGICAL_REAL_OPERATOR(op)                       \
    constexpr bool operator op(const number& other) const noexcept {   \
        if(value_.index() == other.value_.index())           \
            return value_ op other.value_;                   \
        else                                                 \
            return static_cast<real_type>(*this) op          \
                   static_cast<real_type>(other); }          \
    bool operator op(const rational& other) const noexcept { \
        return std::holds_alternative<rational>(value_)      \
            ? std::get<rational>(value_) op other            \
            : static_cast<real_type>(*this) op static_cast<real_type>(other); } \
    template<typename T>                                     \
    constexpr bool operator op(const T& other) const noexcept {        \
        return static_cast<real_type>(*this) op static_cast<real_type>(other); }

    #define _ARITHMATIC_REAL_OPERATOR(op)                                     \
    constexpr inline number operator op(const number& other) const noexcept { \
        if(value_.index() == other.value_.index() && std::holds_alternative<rational>(value_)) \
            return number{std::get<rational>(value_) op std::get<rational>(other.value_)}; \
        else                                                                  \
            return number{static_cast<real_type>(*this) op                    \
                              static_cast<real_type>(other)};  }

    #define _ARITHMATIC_FREE_REAL_OPERATOR(op)                                         \
    template<typename T>                                                               \
    constexpr inline number operator op(const T &left, const number &right) noexcept { \
            return number{left} op right; }

    using real_type = long double;
    using rational_type = long long;
    using rational_base = boost::rational<rational_type>;

    class rational: public rational_base {
    public:
        constexpr rational() = default;
        constexpr rational(const rational_type numerator) noexcept : rational_base(numerator) { }
        rational(const rational_type numerator, const rational_type denominator) noexcept
                : rational_base(numerator, denominator) { }
        constexpr rational(const rational &other) noexcept = default;

        constexpr inline explicit operator double() const noexcept {
            return numerator() / static_cast<double>(denominator());
        }
        constexpr inline explicit operator real_type() const noexcept {
            return numerator() / static_cast<real_type>(denominator());
        }

        _LOGICAL_RATIONAL_OPERATOR(==)
        _LOGICAL_RATIONAL_OPERATOR(>=)
        _LOGICAL_RATIONAL_OPERATOR(<=)
        _LOGICAL_RATIONAL_OPERATOR(>)
        _LOGICAL_RATIONAL_OPERATOR(<)

        _ARITHMATIC_RATIONAL_OPERATOR(+)
        _ARITHMATIC_RATIONAL_OPERATOR(-)
        _ARITHMATIC_RATIONAL_OPERATOR(*)
        _ARITHMATIC_RATIONAL_OPERATOR(/)

        inline std::string to_string() const noexcept {
            return std::to_string(numerator()) + '/' + std::to_string(denominator());
        }

    private:
        constexpr explicit rational(rational_base value) : rational_base(value) { }
    };

    class rational_times_real {
    public:
        constexpr rational_times_real(const rational &rational, const real_type real) noexcept
                : rational_(rational), real_(real) { }
        constexpr rational_times_real(const rational_times_real&) noexcept = default;
        constexpr rational_times_real(rational_times_real&&) noexcept = default;

        inline constexpr explicit operator double() const noexcept {
            return static_cast<double>(static_cast<real_type>(*this));
        }
        inline constexpr explicit operator real_type() const noexcept {
            return static_cast<real_type>(rational_) * real_;
        }

        inline constexpr rational_times_real& operator=(const rational_times_real&) noexcept = default;

        inline std::string to_string() const noexcept {
            return std::to_string(real_) + '*' +
                   std::to_string(rational_.numerator()) + '/' + std::to_string(rational_.denominator());
        }

        inline bool operator==(const rational_times_real& other) const noexcept {
            return rational_ == other.rational_ && real_ == other.real_;
        }
        inline bool operator>=(const rational_times_real& other) const noexcept {
            return static_cast<real_type>(*this) >= static_cast<real_type>(other);
        }
        inline bool operator<=(const rational_times_real& other) const noexcept {
            return static_cast<real_type>(*this) <= static_cast<real_type>(other);
        }
        inline bool operator>(const rational_times_real& other) const noexcept {
            return static_cast<real_type>(*this) > static_cast<real_type>(other);
        }
        inline bool operator<(const rational_times_real& other) const noexcept {
            return static_cast<real_type>(*this) < static_cast<real_type>(other);
        }

    private:
        rational rational_;
        real_type real_;
    };

    namespace internal {
        struct StringVisitor {
            template<typename T>
            inline std::string operator()(const T &r) const { return r.to_string(); }
            inline std::string operator()(const real_type &r) const { return std::to_string(r); }
        };

        struct RealTypeVisitor {
            template<typename T>
            inline constexpr real_type operator()(const T &r) const { return static_cast<real_type>(r); }
        };
    }

    template<class tolerance, typename T1, typename T2>
    constexpr inline bool epsilon_equal(const T1 &left, const T2 &right) noexcept {
        return rational{tolerance::num, tolerance::den} >
                 boost::math::relative_difference(left, right);
    }

    struct number {
    public:
        constexpr number(int integer) noexcept : number(rational{integer}) { }
        constexpr number(long integer) noexcept : number(rational{integer}) { }
        constexpr number(long long integer) noexcept : number(rational{integer}) { }
        constexpr number(unsigned int integer) noexcept : number(rational{integer}) { }
        constexpr number(size_t integer) : number(rational{static_cast<rational_type>(integer)}) {
            if(integer > std::numeric_limits<rational_type>::max()) {
                value_ = static_cast<real_type>(integer);
            }
        }
        constexpr number(const rational &rational) noexcept : value_{rational} { }
        constexpr explicit number(const real_type real) noexcept : value_{real} { }
        constexpr number(rational_times_real value) noexcept : value_{value} { }
        constexpr number(const double real) noexcept : value_{real} { }
        constexpr number(const number& other) noexcept = default;

        constexpr inline explicit operator real_type() const noexcept {
            return std::visit(internal::RealTypeVisitor(), value_);
        }

        constexpr inline explicit operator double() const noexcept {
            return static_cast<double>(static_cast<real_type>(*this));
        }

        constexpr inline explicit operator unsigned int() const noexcept {
            return static_cast<unsigned int>(static_cast<real_type>(*this));
        }

        constexpr inline explicit operator size_t() const noexcept {
            return static_cast<size_t>(static_cast<real_type>(*this));
        }

        inline number& operator=(const number& other) noexcept = default;

        _LOGICAL_REAL_OPERATOR(==)
        _LOGICAL_REAL_OPERATOR(>=)
        _LOGICAL_REAL_OPERATOR(<=)
        _LOGICAL_REAL_OPERATOR(>)
        _LOGICAL_REAL_OPERATOR(<)
        _ARITHMATIC_REAL_OPERATOR(+)
        _ARITHMATIC_REAL_OPERATOR(-)
        _ARITHMATIC_REAL_OPERATOR(*)
        _ARITHMATIC_REAL_OPERATOR(/)

        inline std::string to_string() const noexcept { return std::visit(internal::StringVisitor(), value_); }

        template<typename tolerance=std::pico, typename T>
        inline constexpr bool epsilon_equal(const T &other) const noexcept {
            if(std::holds_alternative<rational>(value_) && std::holds_alternative<rational>(other.value_)) {
                const auto &left = std::get<rational>(value_);
                const auto &right = std::get<rational>(other.value_);

                //abs(a - b) < n/m => m * abs(a - b) < n)
                return tolerance::den * abs(left - right) < tolerance::num;
            } else
                return lightdb::epsilon_equal<tolerance>(static_cast<real_type>(*this), other);
        }

    private:
        std::variant<rational, real_type, rational_times_real> value_;
    };

    _ARITHMATIC_FREE_REAL_OPERATOR(+)
    _ARITHMATIC_FREE_REAL_OPERATOR(-)
    _ARITHMATIC_FREE_REAL_OPERATOR(*)
    _ARITHMATIC_FREE_REAL_OPERATOR(/)

    inline std::string to_string(const number &value) noexcept {
        return value.to_string();
    }

    inline std::string to_string(const rational &value) noexcept {
        return value.to_string();
    }

    inline std::ostream &operator <<(std::ostream &stream, const number &value) noexcept {
        return stream << to_string(value);
    }

    inline std::ostream &operator <<(std::ostream &stream, const rational &value) noexcept {
        return stream << to_string(value);
    }

    inline number round(const number &value) noexcept {
        return {std::lround((real_type)value)};
    }

} // namespace lightdb

#endif //LIGHTDB_NUMBER_H
