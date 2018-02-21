#ifndef LIGHTDB_RATIONAL_H
#define LIGHTDB_RATIONAL_H

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/rational.hpp>
#include <ratio>

namespace lightdb {
    #define _LOGICAL_RATIONAL_OPERATOR(op)                                                       \
    inline bool operator op(const rational& other) const noexcept {                              \
        return static_cast<rational_base>(*this) op static_cast<rational_base>(other); }

    #define _ARITHMATIC_RATIONAL_OPERATOR(op)                                                    \
    inline rational operator op(const rational& other) const {                                   \
        return rational{static_cast<rational_base>(*this) op static_cast<rational_base>(other)}; }


    #define _LOGICAL_REAL_OPERATOR(op)                                                           \
    inline bool operator op(const number& other) const noexcept {                                \
        return type_ == type::rational && other.type_ == type::rational                          \
            ? rational_ op other.rational_                                                       \
            : static_cast<const real_type>(*this) op static_cast<const real_type>(other); }      \
    inline bool operator op(const rational& other) const noexcept {                              \
        return type_ == type::rational                                                           \
            ? rational_ op other                                                                 \
            : static_cast<const real_type>(*this) op static_cast<const real_type>(other); }      \
    template<typename T>                                                                         \
    inline bool operator op(const T& other) const noexcept {                                     \
        return static_cast<const real_type>(*this) op static_cast<const real_type>(other); }

    #define _ARITHMATIC_REAL_OPERATOR(op)                                                        \
    constexpr inline number operator op(const number& other) const noexcept {                    \
        if(type_ == type::rational && other.type_ == type::rational)                             \
            return number{rational_ op other.rational_};                                         \
        else                                                                                     \
            return number{static_cast<const real_type>(*this) op                                 \
                              static_cast<const real_type>(other)};  }

    #define _ARITHMATIC_FREE_REAL_OPERATOR(op)                                                   \
    template<typename T>                                                                         \
    constexpr inline number operator op(const T &left, const number &right) noexcept {           \
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
        constexpr rational(const rational &other) noexcept : rational_base(other) { }

        inline explicit operator double() const noexcept {
            return numerator() / static_cast<double>(denominator());
        }
        inline explicit operator real_type() const noexcept {
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

    template<class tolerance, typename T1, typename T2>
    constexpr inline bool epsilon_equal(const T1 &left, const T2 &right) noexcept {
        return rational{tolerance::num, tolerance::den} > boost::math::relative_difference(left, right);
    }

    struct number {
    public:
        constexpr number(int integer) noexcept : number(rational{integer}) { }
        constexpr number(long integer) noexcept : number(rational{integer}) { }
        constexpr number(long long integer) noexcept : number(rational{integer}) { }
        constexpr number(unsigned int integer) noexcept : number(rational{integer}) { }
        constexpr number(size_t integer) : number(rational{static_cast<rational_type>(integer)}) {
            if(integer > std::numeric_limits<rational_type>::max()) {
                type_ = type::real;
                real_ = integer;
            }
        }
        constexpr number(const rational &rational) noexcept : type_(type::rational), rational_{rational} { }
        constexpr explicit number(const real_type real) noexcept : type_(type::real), real_{real} { }
        constexpr number(const double real) noexcept : type_(type::real), real_{real} { }
        constexpr number(const number &other) noexcept : type_(other.type_), rational_(other.rational_) { }

        constexpr inline explicit operator real_type() const noexcept {
            return type_ == type::rational
                ? static_cast<real_type>(rational_)
                : real_;
        }

        constexpr inline explicit operator unsigned int() const noexcept {
            return static_cast<unsigned int>(static_cast<real_type>(*this));
        }

        constexpr inline explicit operator size_t() const noexcept {
            return static_cast<size_t>(static_cast<real_type>(*this));
        }

        inline number& operator=(const number& other) noexcept {
            type_ = other.type_;
            if(type_ == type::rational)
                rational_ = other.rational_;
            else
                real_ = other.real_;
            return *this;
        }

        _LOGICAL_REAL_OPERATOR(==)
        _LOGICAL_REAL_OPERATOR(>=)
        _LOGICAL_REAL_OPERATOR(<=)
        _LOGICAL_REAL_OPERATOR(>)
        _LOGICAL_REAL_OPERATOR(<)
        _ARITHMATIC_REAL_OPERATOR(+)
        _ARITHMATIC_REAL_OPERATOR(-)
        _ARITHMATIC_REAL_OPERATOR(*)
        _ARITHMATIC_REAL_OPERATOR(/)

        std::string to_string() const noexcept {
            return type_ == type::rational
                ? rational_.to_string()
                : std::to_string(real_);
        }

        template<typename tolerance=std::pico, typename T>
        inline bool epsilon_equal(const T &other) const noexcept {
            return type_ == type::rational && other.type_ == type::rational
                    ? (rational_ - other) < rational{tolerance::num, tolerance::den}
                    : lightdb::epsilon_equal<tolerance>(static_cast<real_type>(*this), other);
        }

    private:
        enum class type {
            rational,
            real
        } type_;
        union {
            rational rational_;
            real_type real_;
        };
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

    constexpr inline number round(const number &value) noexcept {
        return {std::lround((real_type)value)};
    }

} // namespace lightdb

#endif //LIGHTDB_RATIONAL_H
