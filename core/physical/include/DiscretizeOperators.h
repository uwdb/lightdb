#ifndef LIGHTDB_DISCRETIZEOPERATORS_H
#define LIGHTDB_DISCRETIZEOPERATORS_H

#include "PhysicalOperators.h"
#include "EncodeOperators.h"
#include "Functor.h"
#include "Scale.h"
#include <utility>

namespace lightdb::physical {

class GPUDownsampleResolution: public PhysicalOperator, public GPUOperator {
public:
    GPUDownsampleResolution(const LightFieldReference &logical,
                            PhysicalOperatorReference &parent,
                            IntervalGeometry geometry)
            : GPUDownsampleResolution(logical, parent, std::vector<IntervalGeometry>({std::move(geometry)}))
    { }

    GPUDownsampleResolution(const LightFieldReference &logical,
                            PhysicalOperatorReference &parent,
                            std::vector<IntervalGeometry> geometries)
            : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(parent),
              geometries_(std::move(geometries))
    { }

    std::vector<IntervalGeometry>& geometries() { return geometries_; }

private:
    class Runtime: public runtime::GPUUnaryRuntime<GPUDownsampleResolution, GPUDecodedFrameData> {
    public:
        explicit Runtime(GPUDownsampleResolution &physical)
            : runtime::GPUUnaryRuntime<GPUDownsampleResolution, GPUDecodedFrameData>(physical),
              scaler_(this->context()),
              configurationIsValid_(false)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(iterator() != iterator().eos()) {
                auto input = iterator()++;
                Configuration outputConfiguration = downsampled_configuration(input.configuration());
                GPUDecodedFrameData output{outputConfiguration, input.geometry()};

                for(auto &frame: input.frames()) {
                    auto in = frame->cuda();
                    CudaFrameReference out = CudaFrameReference::make<CudaFrame>(outputConfiguration.height, outputConfiguration.width, in->type());

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

        Configuration downsampled_configuration(const Configuration &base) {
            if (configurationIsValid_ && base == lastBaseConfiguration_)
                return lastDownsampledConfiguration_;

            lastBaseConfiguration_ = base;
            configurationIsValid_ = true;

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

            Configuration proposedConfiguration = Configuration{static_cast<unsigned int>(theta_samples),
                          static_cast<unsigned int>(phi_samples),
                          base.max_width, base.max_height,
                          base.bitrate, base.framerate, {}};

            if (proposedConfiguration != lastDownsampledConfiguration_) {
                LOG(INFO) << "Downsampling to " << theta_samples << 'x' << phi_samples;
                lastDownsampledConfiguration_ = proposedConfiguration;
            }

            return lastDownsampledConfiguration_;
        }

        video::GPUScale scaler_;
        Configuration lastDownsampledConfiguration_;
        Configuration lastBaseConfiguration_;
        bool configurationIsValid_;
    };

private:
    std::vector<IntervalGeometry> geometries_;
};

} // lightdb::physical

#endif //LIGHTDB_DISCRETIZEOPERATORS_H
