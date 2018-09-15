#include "AccessDelimiter.h"

namespace lightdb {

    bytestring AccessDelimiter::GetBytes() const {
        bytestring data = Nal::GetBytes();
        bytestring nal_marker = GetNalMarker();
        data.insert(data.begin(), nal_marker.begin(), nal_marker.end());
        return data;
    }
}
