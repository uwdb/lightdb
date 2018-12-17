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
            : GPUDownsampleResolution(logical, parent, std::vector<IntervalGeometry>({std::move(geometry)}))
    { }

    GPUDownsampleResolution(const LightFieldReference &logical,
                            PhysicalLightFieldReference &parent,
                            std::vector<IntervalGeometry> geometries)
            : GPUUnaryOperator(logical, parent,
                               [this]() { return downsampled_configuration(); }),
              geometries_(std::move(geometries)),
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

    std::vector<IntervalGeometry>& geometries() { return geometries_; }

private:
    std::optional<IntervalGeometry> get_geometry(const Dimension dimension) {
        const auto iterator = std::find_if(geometries_.begin(),
                                           geometries_.end(),
                                           [&dimension](const auto &g) {
                                               return g.dimension() == dimension; });
        return iterator != geometries_.end()
                ? std::optional<IntervalGeometry>{*iterator}
                : std::optional<IntervalGeometry>{};

    }
    Configuration downsampled_configuration()
    {
        const auto &base = parent<GPUOperator>().configuration();
        const auto &theta_geometry = get_geometry(Dimension::Theta);
        const auto &phi_geometry = get_geometry(Dimension::Phi);
        auto theta_samples = theta_geometry.has_value() && theta_geometry.value().size().has_value()
                             ? theta_geometry.value().size().value()
                             : base.width;
        auto phi_samples = phi_geometry.has_value() && phi_geometry.value().size().has_value()
                             ? phi_geometry.value().size().value()
                             : base.height;

        CHECK_LE(theta_samples, std::numeric_limits<unsigned int>::max());
        CHECK_LE(phi_samples, std::numeric_limits<unsigned int>::max());

        LOG(INFO) << "Downsampling to " << theta_samples << 'x' << phi_samples;

        return Configuration{static_cast<unsigned int>(theta_samples),
                             static_cast<unsigned int>(phi_samples),
                             base.max_width, base.max_height,
                             base.bitrate, base.framerate};
    }

    std::vector<IntervalGeometry> geometries_;
    lazy<video::GPUScale> scaler_;
};

} // lightdb::physical

#endif //LIGHTDB_DISCRETIZEOPERATORS_H
