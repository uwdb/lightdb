#include "Nal.h"
#include "SequenceParameterSet.h"
#include "PictureParameterSet.h"
#include "VideoParameterSet.h"
#include "SliceSegmentLayer.h"
#include "AccessDelimiter.h"
#include "Opaque.h"

namespace lightdb::hevc {
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
}; //namespace lightdb::hevc
