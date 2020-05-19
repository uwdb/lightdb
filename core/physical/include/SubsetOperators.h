#ifndef LIGHTDB_SUBSETOPERATORS_H
#define LIGHTDB_SUBSETOPERATORS_H

#include "LightField.h"
#include "PhysicalOperators.h"
#include "Codec.h"
#include "Configuration.h"
#include "VideoEncoder.h"
#include "EncodeWriter.h"
#include "VideoEncoderSession.h"
#include "MaterializedLightField.h"
#include <cstdio>
#include <cmath>
#include <utility>

namespace lightdb::physical {

// Just update the physical configuration, don't actual go ahead and copy the subframes
class GPUAngularSubframe: public PhysicalOperator, public GPUOperator, UnaryOperator {
public:
    explicit GPUAngularSubframe(const LightFieldReference &logical,
                                PhysicalOperatorReference &parent)
            : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(parent)
    { }

private:
    class Runtime : public runtime::GPUUnaryRuntime<GPUAngularSubframe, GPUDecodedFrameData> {
    public:
        explicit Runtime(GPUAngularSubframe &physical)
            : runtime::GPUUnaryRuntime<GPUAngularSubframe, GPUDecodedFrameData>(physical),
              configuration_(create_configuration(configuration()))
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if (iterator() != iterator().eos()) {
                auto v = iterator()++;
                return GPUDecodedFrameData{configuration_, v.geometry(), v.frames()};
                //return GPUDecodedFrameData{GetConfiguration(v.configuration()), v.frames()};
            } else
                return {};
        }

    private:
        Configuration create_configuration(const Configuration &base) {
            if((*iterator()).is<InterpolatedGPUDecodedFrameData>())
                throw InvalidArgumentError("Attempt to take frame subset of interpolated data.", "iterator");
            else if(!(*iterator()).geometry().is<EquirectangularGeometry>())
                throw InvalidArgumentError("Attempt to take cropped subset of non-equirectangular frames.", "iterator");
            else if(logical()->volume().bounding().theta() == physical().parent()->logical()->volume().bounding().theta() &&
               logical()->volume().bounding().phi() == physical().parent()->logical()->volume().bounding().phi())
                return base;
            else {
                auto left   = base.width * (logical()->volume().bounding().theta().start() - physical().parent()->logical()->volume().bounding().theta().start()) / number(TWOPI),
                     right  = base.width - (base.width * (physical().parent()->logical()->volume().bounding().theta().end() - logical()->volume().bounding().theta().end()) / number(TWOPI)),
                     top    = base.height * (logical()->volume().bounding().phi().start() - physical().parent()->logical()->volume().bounding().phi().start()) / number(PI),
                     bottom = base.height - (base.height * (physical().parent()->logical()->volume().bounding().phi().end() - logical()->volume().bounding().phi().end()) / number(PI));

                LOG(INFO) << "Subset: " << static_cast<unsigned int>(right - left) << "x" <<
                          static_cast<unsigned int>(bottom - top);
                return Configuration{static_cast<unsigned int>(right - left),
                                     static_cast<unsigned int>(bottom - top),
                                     0, 0,
                                     base.bitrate, base.framerate,
                                     {static_cast<unsigned int>(left),
                                      static_cast<unsigned int>(top)}};

            }
        }

        const Configuration configuration_;
    };

    /*Configuration GetConfigurationTODOremove() {
        const auto &base = parent<GPUOperator>().configuration2();

        if(logical()->volume().bounding().theta() == parent().logical()->volume().bounding().theta() &&
           logical()->volume().bounding().phi() == parent().logical()->volume().bounding().phi())
            return base;
        else {
            LOG(WARNING) << "Not checking for compatible projections or discrete sampling.";
            auto left   = base.width * (logical()->volume().bounding().theta().start() - parent().logical()->volume().bounding().theta().start()) / number(TWOPI),
                 right  = base.width - (base.width * (parent().logical()->volume().bounding().theta().end() - logical()->volume().bounding().theta().end()) / number(TWOPI)),
                 top    = base.height * (logical()->volume().bounding().phi().start() - parent().logical()->volume().bounding().phi().start()) / number(PI),
                 bottom = base.height - (base.height * (parent().logical()->volume().bounding().phi().end() - logical()->volume().bounding().phi().end()) / number(PI));

            LOG(INFO) << "Subset: " << static_cast<unsigned int>(right - left) << "x" <<
                                       static_cast<unsigned int>(bottom - top);
            return Configuration{static_cast<unsigned int>(right - left),
                                 static_cast<unsigned int>(bottom - top),
                                 0, 0,
                                 base.bitrate, base.framerate,
                                 {static_cast<unsigned int>(left),
                                  static_cast<unsigned int>(top)}};

        }
    }*/
};

class GPUEnsureFrameCropped : public PhysicalOperator, public GPUOperator {
public:
    explicit GPUEnsureFrameCropped(const LightFieldReference &logical,
                                   PhysicalOperatorReference &parent)
            : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(parent)
    { }

private:
    class Runtime: public runtime::GPUUnaryRuntime<GPUEnsureFrameCropped, GPUDecodedFrameData> {
    public:
        explicit Runtime(GPUEnsureFrameCropped &physical)
            : runtime::GPUUnaryRuntime<GPUEnsureFrameCropped, GPUDecodedFrameData>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if (iterator() != iterator().eos()) {
                auto decoded = iterator()++;
                std::vector<GPUFrameReference> output;

                for (const auto &frame: decoded.frames()) {
                    output.emplace_back(GPUFrameReference::make<CudaFrame>(*frame));
                    output.back().downcast<CudaFrame>().copy(lock(), *frame->cuda(), decoded.configuration().offset.top, decoded.configuration().offset.left);
                }

                return GPUDecodedFrameData(decoded.configuration(), decoded.geometry(), output);
            } else
                return std::nullopt;
        }
    };
};


class FrameSubset: public PhysicalOperator, public GPUOperator, UnaryOperator {
public:
    explicit FrameSubset(const LightFieldReference &logical,
                         PhysicalOperatorReference &parent)
            : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(parent)
    { }

    explicit FrameSubset(const LightFieldReference &logical,
                         PhysicalOperatorReference &parent,
                         unsigned long delay_frames)
            : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this, delay_frames)),
              GPUOperator(parent)
    { }

private:
    class Runtime: public runtime::GPUUnaryRuntime<FrameSubset, GPUDecodedFrameData> {
    public:
        explicit Runtime(FrameSubset &physical)
            : runtime::GPUUnaryRuntime<FrameSubset, GPUDecodedFrameData>(physical),
              pending_frames_(DefaultPending()),
              delay_frames_(DefaultDelay())
        { }

        Runtime(FrameSubset &physical, unsigned long delay_frames)
            : runtime:: GPUUnaryRuntime<FrameSubset, GPUDecodedFrameData>(physical),
              pending_frames_(DefaultPending()),
              delay_frames_(delay_frames)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if (iterator() != iterator().eos()) {
                auto packet = iterator()++;
                auto &data = packet.downcast<GPUDecodedFrameData>();

                // Got more frames than we're delaying for, so cut the first (delay remaining) frames
                if(delay_frames_ > 0 && delay_frames_ <= data.frames().size()) {
                    data.frames().erase(data.frames().begin(), data.frames().begin() + delay_frames_);
                    delay_frames_ = 0; // Set to zero in case we need to adjust for pending_frames below
                }
                // Got more frames than we're transmitting, so keep only the first (total sent) frames
                if(delay_frames_ == 0 && pending_frames_ < data.frames().size())
                    data.frames().resize(pending_frames_, data.frames()[0]);

                if(delay_frames_ > 0) {
                    // Still delaying?  Deincrement and return an empty sentinal
                    assert(delay_frames_ >= data.frames().size());
                    delay_frames_ -= data.frames().size();
                    return GPUDecodedFrameData{data.configuration(), data.geometry()};
                } else if(pending_frames_ > 0) {
                    // Still transmitting?  Deincrement and return the current set of frames
                    assert(pending_frames_ >= data.frames().size());
                    pending_frames_ -= data.frames().size();
                    return packet;
                } else
                    // Done transmitting?  Return EOS
                    return {};
            } else
                return {};
        }

    private:
        unsigned long DefaultDelay() {
            const auto delay = logical()->volume().bounding().t().start() - physical().parent()->logical()->volume().bounding().t().start();
            const auto delay_frames = std::lround(static_cast<double>(configuration().framerate * delay));

            CHECK_GE(delay, 0);
            CHECK_GE(delay_frames, 0);

            LOG(INFO) << "Delaying output by " << delay_frames << " frames";

            return static_cast<unsigned long>(delay_frames);
        }

        unsigned long DefaultPending() {
            const auto duration = logical()->volume().bounding().t().magnitude();
            const auto pending_frames = std::lround(static_cast<double>(configuration().framerate * duration));

            CHECK_GE(duration, 0);
            CHECK_GE(pending_frames, 0);

            LOG_IF(WARNING, pending_frames == 0) << "Subset op has zero pending frames; optimizer should have just omitted it";
            LOG(INFO) << "Emitting only " << pending_frames << " frames";

            return static_cast<unsigned long>(pending_frames);
        }

        unsigned long pending_frames_;
        unsigned long delay_frames_;
    };
};

}; // namespace lightdb::physical

#endif //LIGHTDB_SUBSETOPERATORS_H
