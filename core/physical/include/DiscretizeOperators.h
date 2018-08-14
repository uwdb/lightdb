#ifndef LIGHTDB_DISCRETIZEOPERATORS_H
#define LIGHTDB_DISCRETIZEOPERATORS_H

#include "PhysicalOperators.h"
#include "EncodeOperators.h"
#include "Functor.h"
#include "Scale.h"
#include <utility>

namespace lightdb::physical {

class GPUDownsampleResolution: public GPUUnaryOperator<GPUDecodedFrameData> {
public:
    GPUDownsampleResolution(const LightFieldReference &logical,
                            PhysicalLightFieldReference &parent,
                            IntervalGeometry geometry)
            : GPUUnaryOperator(logical, parent,
                               [this]() { return downsampled_configuration(); }),
              geometry_(std::move(geometry)),
              scaler_([this]() { return video::GPUScale(context()); })
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(iterator() != iterator().eos()) {
            GPUDecodedFrameData output;
            auto input = iterator()++;

            for(auto &frame: input.frames())
            {
                auto in = frame->cuda();
                CudaFrameReference out = CudaFrameReference::make<CudaFrame>(configuration().height, configuration().width, NV_ENC_PIC_STRUCT_FRAME);

                scaler_->nv12().scale(lock(), in, out);

                output.frames().emplace_back(out);
            }

            return {output};
        } else
            return {};
    }

    const IntervalGeometry& geometry() const { return geometry_; }

private:
    Configuration downsampled_configuration()
    {
        const auto &base = parent<GPUOperator>().configuration();
        auto theta_samples = geometry().dimension() == Dimension::Theta
                             ? geometry().size().value()
                             : base.width;
        auto phi_samples = geometry().dimension() == Dimension::Phi
                             ? geometry().size().value()
                             : base.height;

        CHECK_LE(theta_samples, std::numeric_limits<unsigned int>::max());
        CHECK_LE(phi_samples, std::numeric_limits<unsigned int>::max());

        return Configuration{static_cast<unsigned int>(theta_samples),
                             static_cast<unsigned int>(phi_samples),
                             base.max_width, base.max_height,
                             base.bitrate, base.framerate};
    }

    const IntervalGeometry geometry_;
    lazy<video::GPUScale> scaler_;
};

} // lightdb::physical

#endif //LIGHTDB_DISCRETIZEOPERATORS_H
