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
#include "lazy.h"
#include <cstdio>
#include <utility>

namespace lightdb::physical {

class GPUAngularSubframe: public GPUUnaryOperator<GPUDecodedFrameData> {
public:
    explicit GPUAngularSubframe(const LightFieldReference &logical,
                                PhysicalLightFieldReference &parent)
            : GPUUnaryOperator(logical, parent, [this]() { return GetConfiguration(); })
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if (iterator() != iterator().eos()) {
            return iterator()++;
        } else
            return {};
    }

private:
    Configuration GetConfiguration() {
        const auto &base = parent<GPUOperator>().configuration();

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
    }
};

class FrameSubset: public GPUUnaryOperator<GPUDecodedFrameData> {
public:
    explicit FrameSubset(const LightFieldReference &logical,
                            PhysicalLightFieldReference &parent)
            : FrameSubset(logical, parent, lazy<unsigned long>([this]() { return DefaultDelay(); }))
    { }

    explicit FrameSubset(const LightFieldReference &logical,
                         PhysicalLightFieldReference &parent,
                         unsigned long delay_frames)
            : FrameSubset(logical, parent, lazy<unsigned long>([delay_frames]() { return delay_frames; }))
    { }

    explicit FrameSubset(const LightFieldReference &logical,
                         PhysicalLightFieldReference &parent,
                         lazy<unsigned long> delay_frames)
            : GPUUnaryOperator(logical, parent),
              pending_frames_([this]() { return DefaultPending(); }),
              delay_frames_(std::move(delay_frames))
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if (iterator() != iterator().eos()) {
            auto packet = iterator()++;
            auto &data = packet.downcast<GPUDecodedFrameData>();

            // Got more frames than we're delaying for, so cut the first (delay remaining) frames
            if(delay_frames_ > 0 && delay_frames_ <= data.frames().size()) {
                data.frames().erase(data.frames().begin(), data.frames().begin() + delay_frames_);
                delay_frames_.value() = 0; // Set to zero in case we need to adjust for pending_frames below
            }
            // Got more frames than we're transmitting, so keep only the first (total sent) frames
            if(delay_frames_ == 0 && pending_frames_ < data.frames().size())
                data.frames().resize(pending_frames_, data.frames()[0]);

            if(delay_frames_ > 0) {
                // Still delaying?  Deincrement and return an empty sentinal
                assert(delay_frames_ >= data.frames().size());
                delay_frames_ -= data.frames().size();
                return GPUDecodedFrameData{};
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
        const auto delay = logical()->volume().bounding().t().start() - parent().logical()->volume().bounding().t().start();
        const auto delay_frames = static_cast<unsigned long>(configuration().framerate * delay);

        CHECK_GE(delay, 0);
        CHECK_EQ(static_cast<double>(configuration().framerate * delay), delay_frames);

        return delay_frames;
    }

    unsigned long DefaultPending() {
        const auto duration = logical()->volume().bounding().t().magnitude();
        const auto pending_frames = static_cast<unsigned long>(configuration().framerate * duration);

        CHECK_GE(duration, 0);
        CHECK_EQ(static_cast<double>(configuration().framerate * duration), pending_frames);

        LOG_IF(WARNING, pending_frames == 0) << "Subset op has zero pending frames; optimizer should have just omitted it";

        return pending_frames;
    }

    lazy<unsigned long> pending_frames_;
    lazy<unsigned long> delay_frames_;
};

}; // namespace lightdb::physical

#endif //LIGHTDB_SUBSETOPERATORS_H
