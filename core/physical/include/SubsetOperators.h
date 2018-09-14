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

}; // namespace lightdb::physical

#endif //LIGHTDB_SUBSETOPERATORS_H
