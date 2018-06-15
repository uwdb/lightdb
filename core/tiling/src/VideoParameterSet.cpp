//
// Created by sophi on 4/17/2018.
//

#include <iostream>
#include <string>
#include <cassert>
#include "Nal.h"
#include "VideoParameterSet.h"
#include "Profile.h"
#include "Emulation.h"

using lightdb::utility::BitArray;
using std::copy;

namespace lightdb {

    bytestring VideoParameterSet::GetBytes() const {
        auto nal_marker = GetNalMarker();
        bytestring bytes(nal_marker.size() + data_.size());
        copy(nal_marker.begin(), nal_marker.end(), bytes.begin());
        copy(data_.begin(), data_.end(), bytes.begin() + nal_marker.size());
        return bytes;
    }

    int VideoParameterSet::VPSMaxSubLayersMinus1() const {
        return data_[GetHeaderSize() + kVPSMaxSubLayersMinus1Offset] & kVPSMaxSubLayersMinus1Mask;
    }

    void VideoParameterSet::SetGeneralLevelIDC(int value) {
        // Profile size is in bits, so must be converted to bytes
        data_[kSizeBeforeProfile + profile_size_ / 8 - kGeneralLevelIDCSize] = static_cast<unsigned char>(value);
    }

    int VideoParameterSet::GetGeneralLevelIDC() const {
        return data_[kSizeBeforeProfile + profile_size_ / 8 - kGeneralLevelIDCSize];
    }
}

