#ifndef LIGHTDB_HEADERS_H
#define LIGHTDB_HEADERS_H

#include "StitchContext.h"
#include "SequenceParameterSet.h"
#include "VideoParameterSet.h"
#include "PictureParameterSet.h"

namespace lightdb::hevc {

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
        Headers(const StitchContext &context, std::vector<bytestring> nals);

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
        inline std::shared_ptr<VideoParameterSet> GetVideo() const {
            return std::dynamic_pointer_cast<VideoParameterSet>(headers_[video_]);
        }

        /**
         *
         * @return The SequenceParameterSet header
         */
        inline std::shared_ptr<SequenceParameterSet> GetSequence() const {
            return std::dynamic_pointer_cast<SequenceParameterSet>(headers_[sequence_]);
        }

        /**
         *
         * @return The PictureParameterSet header
         */
        inline std::shared_ptr<PictureParameterSet> GetPicture() const {
            return std::dynamic_pointer_cast<PictureParameterSet>(headers_[picture_]);
        }

        static constexpr unsigned int kNumHeaders = 3u;

    private:
        unsigned int sequence_;
        unsigned int picture_;
        unsigned int video_;
        std::vector<std::shared_ptr<Nal>> headers_;
    };
}; //namespace lightdb::hevc

#endif //LIGHTDB_HEADERS_H
