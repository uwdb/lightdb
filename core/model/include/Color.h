#ifndef LIGHTDB_COLOR_H
#define LIGHTDB_COLOR_H

#include "errors.h"
#include "reference.h"
#include <optional>
#include <memory>
#include <utility>
#include <vector>

namespace lightdb
{
    class YUVColor;
    using bytestring = std::vector<char>;


    class ColorSpace {
    protected:
        ColorSpace() = default;
    };

    class YUVColorSpace: public ColorSpace {
    public:
        typedef YUVColor Color;

        static constexpr const YUVColorSpace instance() { return {}; }

    protected:
        YUVColorSpace() = default;
    };

    class UnknownColorSpace: public ColorSpace {
    public:
        static constexpr const UnknownColorSpace instance() { return {}; }
        //static const UnknownColorSpace Instance;

    protected:
        UnknownColorSpace() = default;
    };

    inline bool operator==(const ColorSpace &left, const ColorSpace &right)
    {
        return &left == &right;
    }

    inline bool operator!=(const ColorSpace &left, const ColorSpace &right)
    {
        return !(left == right);
    }

    class Color {
    public:
        virtual explicit operator YUVColor() const = 0;
        virtual explicit operator bytestring() const = 0;

        template<typename TargetColor>
        explicit operator TargetColor() const { return TargetColor{static_cast<YUVColor>(*this)}; }

        virtual const ColorSpace colorSpace() const = 0;

    protected:
        Color() = default;
    };

    using ColorReference = shared_reference<Color>;

    class YUVColor: public Color {
    public:
        constexpr YUVColor(const unsigned char y, const unsigned char u, const unsigned char v)
            : y_(y), u_(u), v_(v)
        { }
        YUVColor(const YUVColor &color)
            : YUVColor(color.y(), color.u(), color.v())
        { }
        explicit YUVColor(const bytestring &bytes)
            : y_(first_element_and_check_size(bytes)),
              u_(static_cast<unsigned char>(bytes[1])),
              v_(static_cast<unsigned char>(bytes[2]))
        { }

        constexpr const ColorSpace colorSpace() const override { return YUVColorSpace::instance(); }

        explicit operator YUVColor() const override { return *this; }
        explicit operator bytestring() const override {
            return {static_cast<char>(y_), static_cast<char>(u_), static_cast<char>(v_)};
        }

        bool operator !=(const YUVColor& other) const { return !(*this == other); }
        bool operator ==(const YUVColor& other) const {
            return y() == other.y() && u() == other.u() && v() == other.v();
        }

        constexpr unsigned char y() const { return y_; }
        constexpr unsigned char u() const { return u_; }
        constexpr unsigned char v() const { return v_; }

        static constexpr const YUVColor green() { return {149, 43, 21}; }
        static constexpr const YUVColor red() { return {76, 84, 255}; }

    private:
        const unsigned char y_, u_, v_;

        static unsigned char first_element_and_check_size(const bytestring &bytes)
        {
            if(bytes.size() != 3)
                throw InvalidArgumentError("Expected length 3", "bytes"); //TODO
            else
                return static_cast<unsigned char>(bytes[0]);
        }
    };

} // namespace lightdb

#endif //LIGHTDB_COLOR_H
