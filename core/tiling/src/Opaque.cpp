

#include <string>
#include "Opaque.h"
#include "Nal.h"

using std::string;

namespace lightdb {

    bytestring Opaque::GetBytes() const {
        auto data = Nal::GetBytes();
        auto nal_marker = GetNalMarker();
        data.insert(data.begin(), nal_marker.begin(), nal_marker.end());
        return data;
    }

}
