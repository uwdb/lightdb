#ifndef LIGHTDB_INTERPOLATION_H
#define LIGHTDB_INTERPOLATION_H

#include "Geometry.h"
#include "Color.h"
#include <functional>

class LightFieldReference;

namespace lightdb {
namespace interpolation {
    template<typename ColorSpace>
    using interpolator = std::function<typename ColorSpace::Color(
            const LightFieldReference&, const Point6D &point)>;

    //TODO
    static interpolator<YUVColorSpace> NearestNeighbor = [](auto&, auto&) { return YUVColor::Green; };
} // namespace interpolation
} // namespace lightdb

#endif //LIGHTDB_INTERPOLATION_H
