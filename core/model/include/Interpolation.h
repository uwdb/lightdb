#ifndef LIGHTDB_INTERPOLATION_H
#define LIGHTDB_INTERPOLATION_H

#include "Geometry.h"
#include "Color.h"
#include <functional>


namespace lightdb {


namespace interpolation {

class interpolator {
public:
    template<typename ColorSpace=YUVColorSpace>
    typename ColorSpace::Color operator()(const lightdb::LightFieldReference& lightField, const Point6D& point) const {
        return operator()(ColorSpace::Instance, lightField, point);
    }

protected:
    //TODO not happy about doing one heap allocation per interpolation
    virtual const ColorReference operator()(const ColorSpace&, const lightdb::LightFieldReference&, const Point6D&) const = 0;
};

class NearestNeighbor: public interpolator {
protected:
    const ColorReference operator()(const ColorSpace &colorSpace, const lightdb::LightFieldReference&, const Point6D&) const override {
        return ColorReference::make<YUVColor>(YUVColor::green()); //TODO
    }
};

} // namespace interpolation
} // namespace lightdb

#endif //LIGHTDB_INTERPOLATION_H
