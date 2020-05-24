#include "Color.h"

namespace lightdb {
    template<typename TargetColor>
    Color::operator TargetColor() const {
        return TargetColor{static_cast<YUVColor>(*this)};
    }
} // namespace lightdb