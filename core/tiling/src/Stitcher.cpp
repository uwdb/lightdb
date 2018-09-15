//
// Created by sophi on 4/9/2018.
//

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <iterator>

#include "Stitcher.h"
#include "Nal.h"
#include "Context.h"
#include "SliceSegmentLayer.h"
#include "SequenceParameterSet.h"
#include "VideoParameterSet.h"
#include "PictureParameterSet.h"
#include "Headers.h"

using std::list;
using std::vector;
using std::string;
using std::copy;
using std::move;
using std::make_move_iterator;

namespace lightdb {
    
    vector<vector<bytestring>> Stitcher::GetNals() {
        tile_nals_.reserve(tiles_.size());
        for (auto tile : tiles_) {
            vector<bytestring> nals;
            auto zero_count = 0u;
            auto first = true;
            auto start = tile.begin();
            for (auto it = tile.begin(); it != tile.end(); it++) {
                auto c = static_cast<unsigned char>(*it);
                // We found the nal marker "0001"
                if (c == 1 && zero_count >= 3) {
                    // Since each stream will start with 0001, the first segment will always be empty,
                    // so we want to just discard it
                    if (!first) {
                        // -3, not -4, because the constructor is exclusive [first, last)
                        nals.push_back(move(bytestring(make_move_iterator(start), make_move_iterator(it - 3))));
                    } else {
                        first = false;
                    }
                    zero_count = 0;
                    start = it + 1;
                } else if (c == 0) {
                    zero_count++;
                } else {
                    zero_count = 0;
                }
            }
            nals.push_back(move(bytestring(make_move_iterator(start), make_move_iterator(tile.end()))));
            tile_nals_.push_back(nals);
        }
        return tile_nals_;
    }

    vector<bytestring> Stitcher::GetSegmentNals(const unsigned long tile_num, unsigned long *num_bytes, unsigned long *num_keyframes, bool first) {
        auto nals = tile_nals_[tile_num];
        vector<bytestring> segments;
        for (auto nal : nals) {
            if (IsSegment(nal)) {
                if (IsKeyframe(nal) && first) {
                    (*num_keyframes)++;
                }
                *num_bytes += nal.size();
                segments.push_back(move(nal));

            }
        }
        return segments;
    }

    bytestring Stitcher::GetStitchedSegments() {
        headers_.GetSequence()->SetDimensions(context_.GetVideoDimensions());
        headers_.GetSequence()->SetGeneralLevelIDC(120);
        headers_.GetVideo()->SetGeneralLevelIDC(120);
        headers_.GetPicture()->SetTileDimensions(context_.GetTileDimensions());

        auto tile_num = tiles_.size();
        list<vector<bytestring>> segment_nals;
        auto num_segments = 0u;
        unsigned long num_bytes = 0u;
        unsigned long num_keyframes = 0u;

        // First, collect the segment nals from each tile
        for (auto i = 0u; i < tiles_.size(); i++) {
            auto segments = GetSegmentNals(i, &num_bytes, &num_keyframes, i == 0u);
            segment_nals.push_back(segments);
            num_segments += segments.size();
        }

        auto tile_index = 0u;
        vector<bytestring> stitched(num_segments);

        // Next, insert each segment nal from each tile in the appropriate
        // place in the final vector. If we have 3 tiles with 5 segments each,
        // then the segments for tile 0 will go in index 0, 3, 6, 9, 12, the
        // segments for tile 1 will go in index 1, 4, 7, 10, 11, the segments
        // j for tile i will go in index number of tiles * j + i
        for (auto &nals : segment_nals) {
          // Go through the segment nals for a given tile
          for (auto i = 0u; i < nals.size(); i++) {
            stitched[tile_num * i + tile_index] = move(nals[i]);
          }
          // Update the tile index to move to the segment for the next tile
          tile_index++;
        }

        auto addresses = headers_.GetSequence()->GetAddresses();
        auto header_bytes = headers_.GetBytes();
        // Note: this isn't the exact size of the final results since the segments will be extended
        // with the addresses, but this is about as close as we can get. Unfortunately, since it's not
        // the exact size, std::copy cannot be used and we must use insert
        bytestring result(header_bytes.size() * num_keyframes + num_bytes);

        auto it = stitched.begin();
        while (it != stitched.end()) {
          for (auto i = 0; i < tile_num; i++) {
            auto current = Load(context_, *it, headers_);
            if (IsKeyframe(*it)) {
                // We want to insert the header bytes before each GOP, which is indicated by a keyframe.
                // So, the before the first keyframe we encounter in each GOP, we will insert the header
                // bytes
                if (i == 0) {
                    result.insert(result.end(), header_bytes.begin(), header_bytes.end());
                }
            }
            current.SetAddress(addresses[i]);
            auto slice_bytes = current.GetBytes();
            // Insert the bytes of the slice
            result.insert(result.end(), slice_bytes.begin(), slice_bytes.end());
            it++;
          }
        }
        return result;
    }
}
