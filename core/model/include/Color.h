#ifndef VISUALCLOUD_COLOR_H
#define VISUALCLOUD_COLOR_H

class YUVColor;

class Color {
public:
    virtual explicit operator YUVColor() const = 0;
    //virtual Color& Null() const = 0;

protected:
    Color() { }
};

class YUVColor: public Color {
public:
    YUVColor(unsigned char y, unsigned char u, unsigned char v)
            : y_(y), u_(u), v_(v)
    { }

    explicit operator YUVColor() const override { return *this; }
    bool operator ==(const YUVColor& other) const { return y() == other.y() && u() == other.u() && v() == other.v() && this != &Null; }
    bool operator !=(const YUVColor& other) const { return !(*this == other); }

    unsigned char y() const { return y_; }
    unsigned char u() const { return u_; }
    unsigned char v() const { return v_; }

    static const YUVColor Null;
    static const YUVColor Green;

private:
    unsigned char y_, u_, v_;
};

/*class NullColor: public Color {
public:
    explicit operator YUVColor() const override { throw std::runtime_error("TODO"); }

    static const NullColor& Instance() { return instance_; }

private:
    NullColor() { }
    static const NullColor instance_;
};*/


class ColorSpace {
};

class YUVColorSpace: public ColorSpace {
public:
    typedef YUVColor Color;

    static const YUVColorSpace Instance;

protected:
    YUVColorSpace() { }
};

/*class LightField {
protected:
    LightField() { }

public:
};
*/

#endif //VISUALCLOUD_COLOR_H
