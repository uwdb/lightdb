#ifndef LIGHTDB_INTERPOLATION_H
#define LIGHTDB_INTERPOLATION_H

#include "Geometry.h"
#include "Color.h"
#include <functional>

template<typename ColorSpace>
class LightFieldReference;

namespace lightdb {
    //TODO move this into interpolation namespace
    template<typename ColorSpace>
    using interpolator = std::function<typename ColorSpace::Color(
            const LightFieldReference<ColorSpace>&, const Point6D &point)>;

    namespace interpolation {
        //TODO
        static interpolator<YUVColorSpace> NearestNeighbor = [](auto&, auto&) { return YUVColor::Green; };
    } // namespace interpolation
} // namespace lightdb

#endif //LIGHTDB_INTERPOLATION_H
