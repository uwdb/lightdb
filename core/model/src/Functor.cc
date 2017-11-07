#include <mutex>
#include "LightField.h"

namespace visualcloud {
    const YUVColorSpace::Color Greyscale::operator()(const LightField<YUVColorSpace> &field,
                                                     const Point6D &point) const {
        return YUVColor{field.value(point).y(), 0, 0};
    }

    Greyscale::operator const FrameTransform() const {
        return [](VideoLock& lock, Frame& frame) -> Frame& {
            std::scoped_lock{lock};

            auto uv_offset = frame.width() * frame.height();
            auto uv_height = frame.height() / 2;

            assert(cuMemsetD2D8(frame.handle() + uv_offset,
                                frame.pitch(),
                                128,
                                frame.width(),
                                uv_height) == CUDA_SUCCESS);
            return frame;
        };
    };

    const YUVColorSpace::Color Identity::operator()(const LightField<YUVColorSpace> &field,
                                                     const Point6D &point) const {
        return field.value(point);
    }

    Identity::operator const FrameTransform() const {
        return [](VideoLock&, Frame& frame) -> Frame& {
            return frame;
        };
    };
}; // namespace visualcloud