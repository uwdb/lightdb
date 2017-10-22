#ifndef VISUALCLOUD_INTERPOLATION_H
#define VISUALCLOUD_INTERPOLATION_H

#include "Geometry.h"
#include "Color.h"
#include <functional>

template<typename ColorSpace>
class LightFieldReference;

namespace visualcloud {
    template<typename ColorSpace>
    using interpolator = std::function<typename ColorSpace::Color(
            const LightFieldReference<ColorSpace>&, const Point6D &point)>;
} // namespace visualcloud

#endif //VISUALCLOUD_INTERPOLATION_H
