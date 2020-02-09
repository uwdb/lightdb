#ifndef LIGHTDB_GREYSCALE_H
#define LIGHTDB_GREYSCALE_H

#include "Functor.h"

namespace lightdb {
    class Greyscale : public functor::unaryfunctor {
        class GPU : public functor::unaryfunction {
        public:
            GPU() : functor::unaryfunction(physical::DeviceType::GPU, Codec::raw(), true) {}

            shared_reference <LightField> operator()(LightField &input) override {
                auto &data = dynamic_cast<physical::GPUDecodedFrameData&>(input);

                for(auto& frame: data.frames()) {
                    //TODO add NV12 helper utilities
                    auto uv_offset = frame->width() * frame->height();
                    auto uv_height = frame->height() / 2;
                    auto cuda = frame->cuda();

                    CHECK_EQ(cuMemsetD2D8(cuda->handle() + uv_offset,
                                          cuda->pitch(),
                                          128,
                                          cuda->width(),
                                          uv_height), CUDA_SUCCESS);
                }

                return data;
            }
        };

    public:
        Greyscale() : functor::unaryfunctor("Greyscale", GPU()) {}
    };

    static Greyscale Greyscale{};
} //namespace lightdb

#endif //LIGHTDB_GREYSCALE_H
