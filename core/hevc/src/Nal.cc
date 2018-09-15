#include "Nal.h"
#include "NalType.h"
#include "SequenceParameterSet.h"
#include "PictureParameterSet.h"
#include "VideoParameterSet.h"
#include "SliceSegmentLayer.h"
#include "Opaque.h"
#include "AccessDelimiter.h"
#include <iostream>

namespace lightdb {

    class Headers;

    Nal::Nal(const Context &context, const bytestring &data)
            : context_(context),
              byte_data_(data),
              type_(PeekType(data)),
              is_header_(IsHeader()) {
        CHECK_EQ(ForbiddenZero(), 0);
    }

    bytestring Nal::GetBytes() const {
        return byte_data_;
    }

    Context Nal::GetContext() const {
        return context_;
    }

    bool Nal::IsSequence() const {
        return type_ == NalUnitSPS;
    }

    bool Nal::IsVideo() const {
        return type_ == NalUnitVPS;
    }

    bool Nal::IsPicture() const {
        return type_ == NalUnitPPS;
    }
    bool Nal::IsHeader() const {
        return type_ == NalUnitSPS || type_ == NalUnitPPS || type_ == NalUnitVPS;
    }

    unsigned int Nal::ForbiddenZero() const {
      return byte_data_[kForbiddenZeroIndex] & kForbiddenZeroMask;
    }

    bytestring GetNalMarker() {
        return bytestring{0, 0, 1};
        /*bytestring nal_marker(3);
        nal_marker[0] = 0x00;
        nal_marker[1] = static_cast<char>(0x00);
        nal_marker[2] = static_cast<char>(0x01);
        return nal_marker;*/
    }

    bool IsSegment(const bytestring &data) {
        auto type = PeekType(data);
        return type == NalUnitCodedSliceIDRWRADL ||
        type == NalUnitCodedSliceTrailR;
    }

    bool IsKeyframe(const bytestring &data) {
        return PeekType(data) == NalUnitCodedSliceIDRWRADL;
    }

    size_t GetHeaderSize() {
        return kNalHeaderSize - kNalMarkerSize;
    }

    size_t GetHeaderSizeInBits() {
        return GetHeaderSize() * CHAR_BIT;
    }

    unsigned int PeekType(const bytestring &data) {
        assert(!data.empty());
        return (static_cast<unsigned char>(data[0]) & 0x7Fu) >> 1;
    }


    std::shared_ptr<Nal> Load(const Context &context, const bytestring &data) {
        switch(PeekType(data)) {
            case NalUnitSPS:
                return std::make_shared<SequenceParameterSet>(context, data);
            case NalUnitPPS:
                return std::make_shared<PictureParameterSet>(context, data);
            case NalUnitVPS:
                return std::make_shared<VideoParameterSet>(context, data);
            case NalUnitAccessUnitDelimiter:
                return std::make_shared<AccessDelimiter>(context, data);
            default:
                return std::make_shared<Opaque>(context, data);
        }
    }

    SliceSegmentLayer Load(const Context &context, const bytestring &data, const Headers &headers) {
        switch(PeekType(data)) {
            case NalUnitCodedSliceIDRWRADL:
                return IDRSliceSegmentLayer(context, data, headers);
            case NalUnitCodedSliceTrailR:
                return TrailRSliceSegmentLayer(context, data, headers);
            default:
                throw InvalidArgumentError(std::string("Unrecognized SliceSegmentLayer type ") + std::to_string(PeekType(data)), "data");
        }
    }
}
