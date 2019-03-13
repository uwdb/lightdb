#ifndef LIGHTDB_DISCRETIZEOPERATORS_H
#define LIGHTDB_DISCRETIZEOPERATORS_H

#include "PhysicalOperators.h"
#include "EncodeOperators.h"
#include "Functor.h"
#include "Scale.h"
#include <utility>

namespace lightdb::physical {

class GPUDownsampleResolution: public GPUUnaryOperator {
public:
    GPUDownsampleResolution(const LightFieldReference &logical,
                            PhysicalLightFieldReference &parent,
                            IntervalGeometry geometry)
            : GPUDownsampleResolution(logical, parent, std::vector<IntervalGeometry>({std::move(geometry)}))
    { }

    GPUDownsampleResolution(const LightFieldReference &logical,
                            PhysicalLightFieldReference &parent,
                            std::vector<IntervalGeometry> geometries)
            : GPUUnaryOperator(logical, parent, runtime::make<Runtime>(*this)),
              geometries_(std::move(geometries))
    { }

    std::vector<IntervalGeometry>& geometries() { return geometries_; }

private:
    class Runtime: public GPUUnaryOperator::Runtime<GPUDownsampleResolution, GPUDecodedFrameData> {
    public:
        explicit Runtime(GPUDownsampleResolution &physical)
            : GPUUnaryOperator::Runtime<GPUDownsampleResolution, GPUDecodedFrameData>(physical),
              scaler_(context())
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterator() != iterator().eos()) {
                auto input = iterator()++;
                GPUDecodedFrameData output{input.configuration()};

                for(auto &frame: input.frames()) {
                    auto in = frame->cuda();
                    CudaFrameReference out = CudaFrameReference::make<CudaFrame>(*frame);

                    scaler_.nv12().scale(lock(), in, out);

                    output.frames().emplace_back(out);
                }

                return {output};
            } else
                return {};
        }

    private:
        std::optional<IntervalGeometry> get_geometry(const Dimension dimension) {
            const auto iterator = std::find_if(physical().geometries().begin(),
                                               physical().geometries().end(),
                                               [&dimension](const auto &g) {
                                                   return g.dimension() == dimension; });
            return iterator != physical().geometries().end()
                   ? std::optional<IntervalGeometry>{*iterator}
                   : std::optional<IntervalGeometry>{};

        }

        Configuration downsampled_configuration() {
            const auto &base = (*iterator()).configuration();
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
                                 base.bitrate, base.framerate, {}};
        }

        video::GPUScale scaler_;
    };

private:
    std::vector<IntervalGeometry> geometries_;
};

} // lightdb::physical

#endif //LIGHTDB_DISCRETIZEOPERATORS_H
