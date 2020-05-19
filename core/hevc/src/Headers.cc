#include "Headers.h"

namespace lightdb::hevc {

	Headers::Headers(const StitchContext &context, std::vector<bytestring> nals)  {
	    auto i = 0u;

	    // No need to check if it < nals.end() since any well formed stream
        // is guaranteed to have three headers
		for (auto it = nals.begin(); i < kNumHeaders; it++) {
	  		auto current_nal = Load(context, *it);
	  		if (current_nal->IsHeader()) {
                headers_.push_back(current_nal);
                if (current_nal->IsSequence()) {
                    sequence_ = i++;
                } else if (current_nal->IsPicture()) {
                    picture_ = i++;
                } else if (current_nal->IsVideo()) {
                    video_ = i++;
                }
            }
		}
		CHECK_EQ(headers_.size(), kNumHeaders);
	}

	bytestring Headers::GetBytes() const {
		bytestring bytes;

	    for (const auto &header : headers_) {
	    	auto header_bytes = header->GetBytes();
	    	bytes.insert(bytes.end(), header_bytes.begin(), header_bytes.end());
	    }
	    return bytes;
	}
}; //namespace lightdb::hevc
