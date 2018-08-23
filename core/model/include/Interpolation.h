#ifndef LIGHTDB_INTERPOLATION_H
#define LIGHTDB_INTERPOLATION_H

#include "Geometry.h"
#include "Color.h"
#include <functional>


namespace lightdb::interpolation {

class interpolator {
public:
    template<typename ColorSpace=YUVColorSpace>
    typename ColorSpace::Color operator()(const lightdb::LightFieldReference& lightField, const Point6D& point) const {
        return operator()(ColorSpace::Instance, lightField, point);
    }

    const std::string& name() const { return name_; }

protected:
    explicit interpolator(std::string name)
            : name_(std::move(name))
    { }

    //TODO not happy about doing one heap allocation per interpolation
    virtual const ColorReference operator()(const ColorSpace&, const lightdb::LightFieldReference&, const Point6D&) const = 0;

private:
    const std::string name_;
};

using InterpolatorReference = shared_reference<interpolator>;

class Linear: public interpolator {
public:
    Linear() : interpolator("linear") { }

protected:
    const ColorReference operator()(const ColorSpace &colorSpace, const lightdb::LightFieldReference&, const Point6D&) const override {
        return ColorReference::make<YUVColor>(YUVColor::green()); //TODO
}
};

} // namespace lightdb::interpolation

#endif //LIGHTDB_INTERPOLATION_H
