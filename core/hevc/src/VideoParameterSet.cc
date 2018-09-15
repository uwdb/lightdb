#include "Nal.h"
#include "VideoParameterSet.h"

namespace lightdb {

    bytestring VideoParameterSet::GetBytes() const {
        auto nal_marker = GetNalMarker();
        bytestring bytes(nal_marker.size() + data_.size());
        copy(nal_marker.begin(), nal_marker.end(), bytes.begin());
        copy(data_.begin(), data_.end(), bytes.begin() + nal_marker.size());
        return bytes;
    }

    unsigned int VideoParameterSet::VPSMaxSubLayersMinus1() const {
        return data_[GetHeaderSize() + kVPSMaxSubLayersMinus1Offset] & kVPSMaxSubLayersMinus1Mask;
    }

    void VideoParameterSet::SetGeneralLevelIDC(const unsigned char value) {
        // Profile size is in bits, so must be converted to bytes
        data_[kSizeBeforeProfile + profile_size_ / 8 - kGeneralLevelIDCSize] = value;
    }

    unsigned int VideoParameterSet::GetGeneralLevelIDC() const {
        return static_cast<unsigned char>(data_[kSizeBeforeProfile + profile_size_ / 8 - kGeneralLevelIDCSize]);
    }
}

