#ifndef LIGHTDB_HEADERS_H
#define LIGHTDB_HEADERS_H

#include "Context.h"
#include "SequenceParameterSet.h"
#include "VideoParameterSet.h"
#include "PictureParameterSet.h"

namespace lightdb {

    class VideoParameterSet;
    class PictureParameterSet;
    class SequenceParameterSet;

    class Headers {
    public:

        /**
         * Extracts the three headers from nals
         * @param context The context of the nals
         * @param nals The byte streams of the nals
         */
        Headers(Context &context, std::vector<bytestring> nals);

        /**
         *
         * @return The bytes of the headers, in the order that the headers were provided
         * in the "nals" argument to the constructor
         */
        bytestring GetBytes() const;

        /**
         *
         * @return The VideoParameterSet header
         */
        std::shared_ptr<VideoParameterSet> GetVideo() const;

        /**
         *
         * @return The SequenceParameterSet header
         */
        std::shared_ptr<SequenceParameterSet> GetSequence() const;

        /**
         *
         * @return The PictureParameterSet header
         */
        std::shared_ptr<PictureParameterSet> GetPicture() const;

        static const int kNumHeaders = 3;

    private:
        int sequence_;
        int picture_;
        int video_;
        std::vector<std::shared_ptr<Nal>> headers_;
    };
}

#endif //LIGHTDB_HEADERS_H
