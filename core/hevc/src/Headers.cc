//
// Created by sophi on 4/10/2018.
//

#include <vector>
#include <string>
#include <cassert>
#include <memory>
#include <iostream>

#include "Headers.h"
#include "Nal.h"
#include "SequenceParameterSet.h"
#include "VideoParameterSet.h"
#include "PictureParameterSet.h"

using std::vector;
using std::shared_ptr;

namespace lightdb {

	Headers::Headers(Context &context, vector<bytestring> nals)  {
	    auto i = 0;
	    // No need to check if it < nals.end() since any well formed stream
        // is guaranteed to have three headers
		for (auto it = nals.begin(); i < kNumHeaders; it++) {
	  		shared_ptr<Nal> current_nal = Load(context, *it);
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
		assert(headers_.size() == kNumHeaders);
	}

	bytestring Headers::GetBytes() const {
		bytestring bytes;
	    for (auto header : headers_) {
	    	auto header_bytes = header->GetBytes();
	    	bytes.insert(bytes.end(), header_bytes.begin(), header_bytes.end());
	    }
	    return bytes;
	}

    shared_ptr<PictureParameterSet> Headers::GetPicture() const {
	    return std::dynamic_pointer_cast<PictureParameterSet>(headers_[picture_]);
	}

    shared_ptr<SequenceParameterSet> Headers::GetSequence() const {
		return std::dynamic_pointer_cast<SequenceParameterSet>(headers_[sequence_]);
	}

    shared_ptr<VideoParameterSet> Headers::GetVideo() const {
		return std::dynamic_pointer_cast<VideoParameterSet>(headers_[video_]);
	}
}
