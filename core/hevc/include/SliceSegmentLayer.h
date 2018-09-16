#ifndef LIGHTDB_SLICESEGMENTLAYER_H
#define LIGHTDB_SLICESEGMENTLAYER_H

#include "Nal.h"
#include "Headers.h"
#include "BitStream.h"
#include "BitArray.h"
#include "Emulation.h"


namespace lightdb {
    // Defined in 7.3.6.1 and 7.3.2.9
    class SliceSegmentLayer : public Nal {
    public:

        /**
        * Interprets data as a byte stream representing a SliceSegmentLayer
        * @param context The context surrounding the Nal
        * @param data The byte stream
        * @param headers The headers associated with this segment
        */
        SliceSegmentLayer(const Context &context, const bytestring &data, Headers headers)
                : Nal(context, data),
                  data_(RemoveEmulationPrevention(data, GetHeaderSize(), kMaxHeaderLength)),
                  headers_(std::move(headers)),
                  metadata_(data_.begin(), data_.begin() + GetHeaderSizeInBits()),
                  address_(0)
        { }

        /**
        * Sets the address of this segment to be address. Should only be called once -
        * if the address is set, should not be reset
        * @param address The new address of this segment
        */
        void SetAddress(size_t address);

        /**
        *
        * @return This segment's address
        */
        inline size_t GetAddress() const {
            return address_;
        }

        /**
        *
        * @return A string with the bytes of this Nal
        */
        inline bytestring GetBytes() const override {
            return AddEmulationPreventionAndMarker(data_, GetHeaderSize(), kMaxHeaderLength);
        }

        SliceSegmentLayer(const SliceSegmentLayer& other) = default;
        SliceSegmentLayer(SliceSegmentLayer&& other) = default;
        ~SliceSegmentLayer() = default;

    protected:
        inline utility::BitStream& GetBitStream() {
            return metadata_;
        }

    private:
        utility::BitArray data_;
        const Headers headers_;
        utility::BitStream metadata_;
        size_t address_;

        static constexpr unsigned int kFirstSliceFlagOffset = 0;
        static constexpr unsigned int kMaxHeaderLength = 24;
    };

    class IDRSliceSegmentLayer : public SliceSegmentLayer {
    public:

        /**
        * Interprets data as a byte stream representing an IDRSliceSegmentLayer
        * @param context The context surrounding the Nal
        * @param data The byte stream
        * @param headers The headers associated with this segment
        */
        IDRSliceSegmentLayer(const Context &context, const bytestring &data, const Headers &headers);
    };

    class TrailRSliceSegmentLayer : public SliceSegmentLayer {
    public:

        /**
        * Interprets data as a byte stream representing a TrailRSliceSegmentLayer
        * @param context The context surrounding the Nal
        * @param data The byte stream
        * @param headers The headers associated with this segment
        */
        TrailRSliceSegmentLayer(const Context &context, const bytestring &data, const Headers &headers);

    };
}

#endif //LIGHTDB_SLICESEGMENTLAYER_H
