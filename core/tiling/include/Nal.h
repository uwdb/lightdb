//
// Created by sophi on 4/9/2018.
//

#ifndef LIGHTDB_NAL_H
#define LIGHTDB_NAL_H

#include <cstddef>
#include <string>
#include <vector>
#include <climits>
#include <memory>

#include "Context.h"

using bytestring = std::vector<char>;

namespace lightdb {

    class Headers;
    class SliceSegmentLayer;

    class Nal {
     public:

        /**
         * Creates a new Nal
         * @param context The context of the Nal
         * @param data The bytes representing the Nal
         */
        Nal(const Context &context, const bytestring &data);

        /**
         *
         * @return The context associated with this Nal
         */
        Context GetContext() const;

        /**
         *
         * @return True if the nal represents a SequenceParameterSet, false otherwise
         */
        bool IsSequence() const;

        /**
         *
         * @return True if the nal represents a PictureParameterSet, false otherwise
         */
        bool IsPicture() const;

        /**
         *
         * @return True if the nal represents a VideoParameterSet, false otherwise
         */
        bool IsVideo() const;

        /**
         *
         * @return True if the nal represents a header, false otherwise
         */
        bool IsHeader() const;

        /**
         *
         * @return The bytestring that represents this Nal
         */
        virtual bytestring GetBytes() const;

     private:

        int ForbiddenZero() const;

        static const int kForbiddenZeroIndex = 0;
        static const int kForbiddenZeroMask = 0x80;

        Context context_;
        bytestring byte_data_;
        bool is_header_;
        int type_;
    };

/**
 *
 * @return The size of the header for a Nal in bytes
 */
size_t GetHeaderSize();

/**
 *
 * @return The size of the header for a Nal in bits
 */
size_t GetHeaderSizeInBits();

/**
 *
 * @param data The byte stream
 * @return The type of the byte stream as defined in NalType.h
 */
int PeekType(const bytestring &data);

/**
 *
 * @param data The byte stream
 * @return True if the byte stream represents a segment, false otherwise
 */
bool IsSegment(const bytestring &data);

/**
 *
 * @param data The byte stream
 * @return True if the byte stream represents a key frame, false otherwise
 */
bool IsKeyframe(const bytestring &data);


/**
 * @return A string representing a nal marker
 */
bytestring GetNalMarker();

/**
 * Returns a Nal with type based on the value returned by PeekType on data. Since this takes no
 * headers, the Nal cannot be a SliceSegmentLayer. The Nal is instantiated on the heap
 * @param context The context surrounding the data
 * @param data The byte stream
 * @return A Nal with the correct type
 */
std::shared_ptr<Nal> Load(const Context &context, const bytestring &data);

/**
 * Returns a Nal with type based on the value returned by PeekType on data
 * @param context The context surrounding the data
 * @param data The byte stream
 * @param headers The headers of the byte stream (necessary when the byte stream represents a
 * SliceSegmentLayer)
 * @return A Nal with the correct type
 */
SliceSegmentLayer Load(const Context &context, const bytestring &data, const Headers &headers);

static const int kNalHeaderSize = 5;
static const int kNalMarkerSize = 3;
}

#endif //LIGHTDB_NAL_H
