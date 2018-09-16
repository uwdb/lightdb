#ifndef LIGHTDB_NAL_H
#define LIGHTDB_NAL_H

#include "Context.h"
#include "Encoding.h"
#include "NalType.h"

namespace lightdb::hevc {

    class Headers;
    class SliceSegmentLayer;

    /**
     *
     * @param data The byte stream
     * @return The type of the byte stream as defined in NalType.h
     */
    inline unsigned int PeekType(const bytestring &data) {
        assert(!data.empty());
        return (static_cast<unsigned char>(data[0]) & 0x7Fu) >> 1;
    }

    class Nal {
     public:

        /**
         * Creates a new Nal
         * @param context The context of the Nal
         * @param data The bytes representing the Nal
         */
        Nal(Context context, const lightdb::bytestring &data)
                : context_(std::move(context)),
                  byte_data_(data),
                  type_(PeekType(data)),
                  is_header_(IsHeader()) {
            CHECK_EQ(ForbiddenZero(), 0);
        }

        /**
         *
         * @return The context associated with this Nal
         */
        inline const Context& GetContext() const {
            return context_;
        }

        /**
         *
         * @return True if the nal represents a SequenceParameterSet, false otherwise
         */
        inline bool IsSequence() const {
            return type_ == NalUnitSPS;
        }

        /**
         *
         * @return True if the nal represents a PictureParameterSet, false otherwise
         */
        inline bool IsPicture() const {
            return type_ == NalUnitPPS;
        }

        /**
         *
         * @return True if the nal represents a VideoParameterSet, false otherwise
         */
        inline bool IsVideo() const {
            return type_ == NalUnitVPS;
        }

        /**
         *
         * @return True if the nal represents a header, false otherwise
         */
        inline bool IsHeader() const {
            return IsSequence() || IsPicture() || IsVideo();
        }

        /**
         *
         * @return The bytestring that represents this Nal
         */
        inline virtual bytestring GetBytes() const {
            return byte_data_;
        }

        /**
 * @return A string representing a nal marker
 */
        inline bytestring GetNalMarker() {
            return bytestring{0, 0, 1};
        }

        static constexpr std::array<char, 3> kNalMarker{0, 0, 1};
        static constexpr unsigned int kNalHeaderSize = 5u;
        static constexpr unsigned int kNalMarkerSize = kNalMarker.size();

     private:

        inline unsigned int ForbiddenZero() const {
            return byte_data_[kForbiddenZeroIndex] & kForbiddenZeroMask;
        }

        static constexpr const unsigned int kForbiddenZeroIndex = 0u;
        static constexpr const unsigned int kForbiddenZeroMask = 0x80u;

        const Context context_;
        const bytestring byte_data_;
        const unsigned int type_;
        const bool is_header_;
    };

/**
 *
 * @return The size of the header for a Nal in bytes
 */
inline size_t GetHeaderSize() {
    return Nal::kNalHeaderSize - Nal::kNalMarkerSize;
}

/**
 *
 * @return The size of the header for a Nal in bits
 */
inline size_t GetHeaderSizeInBits() {
    return GetHeaderSize() * CHAR_BIT;
}

/**
 *
 * @param data The byte stream
 * @return True if the byte stream represents a segment, false otherwise
 */
inline bool IsSegment(const bytestring &data) {
    auto type = PeekType(data);
    return type == NalUnitCodedSliceIDRWRADL ||
           type == NalUnitCodedSliceTrailR;
}

/**
 *
 * @param data The byte stream
 * @return True if the byte stream represents a key frame, false otherwise
 */
inline bool IsKeyframe(const bytestring &data) {
    return PeekType(data) == NalUnitCodedSliceIDRWRADL;
}


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

}; //namespace lightdb::hevc

#endif //LIGHTDB_NAL_H
