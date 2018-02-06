#ifndef LIGHTDB_FRAMERATEALIGNMENT_H
#define LIGHTDB_FRAMERATEALIGNMENT_H

#include "Configuration.h"

class FrameRateAlignment {
public:
    FrameRateAlignment(const Configuration::FrameRate &encodeRate, const Configuration::FrameRate &decodeRate)
        : FrameRateAlignment(encodeRate.fps() / decodeRate.fps())
    { }

    explicit FrameRateAlignment(const double ratio)
            : ratio(ratio)
    { }

    unsigned int dropOrDuplicate(const size_t decodedFrames, const size_t encodedFrames) {
        if (ratio < 1.f) {
            // Potentially need to drop frame
            return decodedFrames * ratio < (encodedFrames + 1) ? -1 : 0;
        } else if (ratio > 1.f) {
            // May need to duplicate frames
            auto duplicate = 0;
            while(decodedFrames * ratio > encodedFrames + duplicate + 1)
                duplicate++;

            return duplicate;
        } else {
            return 0;
        }
    }

private:
    const double ratio;
};

#endif //LIGHTDB_FRAMERATEALIGNMENT_H
