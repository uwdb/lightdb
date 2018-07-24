#ifndef LIGHTDB_ENCODEOPERATORS_H
#define LIGHTDB_ENCODEOPERATORS_H

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

class GPUEncode : public GPUUnaryOperator<GPUDecodedFrameData> {
public:
    static constexpr size_t kDefaultGopSize = 30;

    explicit GPUEncode(const LightFieldReference &logical,
                       PhysicalLightFieldReference &parent,
                       Codec codec,
                       const unsigned int gop_size=kDefaultGopSize)
            : GPUUnaryOperator(logical, parent),
              codec_(std::move(codec)),
              encodeConfiguration_([this, gop_size]() {
                  return EncodeConfiguration{configuration(), codec_.nvidiaId().value(), gop_size}; }),
              encoder_([this]() { return VideoEncoder(context(), encodeConfiguration_, lock()); }),
              encodeSession_([this]() { return VideoEncoderSession(encoder_, writer_); }),
              writer_([this]() { return MemoryEncodeWriter(encoder_->api()); }) {}

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if (iterator() != iterator().eos()) {
            auto decoded = iterator()++;

            for (const auto &frame: decoded.frames())
                encodeSession_->Encode(*frame);

            //TODO think this should move down just above nullopt
            // Did we just reach the end of the decode stream?
            if (iterator() == iterator().eos())
                // If so, flush the encode queue and end this op too
                encodeSession_->Flush();

            return {CPUEncodedFrameData(codec_, writer_->dequeue())};
        } else
            return std::nullopt;
    }

private:
    const Codec codec_;
    lazy <EncodeConfiguration> encodeConfiguration_;
    lazy <VideoEncoder> encoder_;
    lazy <VideoEncoderSession> encodeSession_;
    lazy <MemoryEncodeWriter> writer_;
};

//TODO There's nothing "Encodey" about this operation, just make an IdentityOp
class IdentityEncode: public PhysicalLightField {
public:
    IdentityEncode(const LightFieldReference &logical,
                   PhysicalLightFieldReference &parent)
            : PhysicalLightField(logical, {parent}, physical::DeviceType::CPU)
    { }

    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(iterators()[0] != iterators()[0].eos()) {
            return iterators()[0]++;
        } else
            return {};
    }
};

}; // namespace lightdb::physical

#endif //LIGHTDB_ENCODEOPERATORS_H
