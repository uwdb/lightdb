#include "PictureParameterSet.h"
#include "Emulation.h"
#include "Golombs.h"

namespace lightdb::hevc {
    void PictureParameterSet::SetTileDimensions(const std::pair<unsigned long, unsigned long>& dimensions, const bool loop_filter_enabled) {
        if (dimensions.first == tile_dimensions_.first &&
            dimensions.second == tile_dimensions_.second) {
            return;
        }

        assert(dimensions.first > 0 && dimensions.second > 0);
        assert(!metadata_.GetValue("tiles_enabled_flag"));
        assert(tile_dimensions_.first == 0 && tile_dimensions_.second == 0);

        // The dimensions are width first, then height, so we must reverse the
        // order from our height, width
        // Next comes a one, and then a 1 if the filter is enabled, 0 if not
        std::vector<unsigned long> new_dimensions{dimensions.second - 1, dimensions.first - 1};

        auto dimensions_bits = EncodeGolombs(new_dimensions);
        // We want to insert the 1 bit 1
        dimensions_bits.Insert(dimensions_bits.size(), 1, 1);

        if (loop_filter_enabled) {
            dimensions_bits.Insert(dimensions_bits.size(), 1, 1);
        } else {
            dimensions_bits.Insert(dimensions_bits.size(), 0, 1);
        }

        // Set the tiles enabled flag to true
        data_[metadata_.GetValue("tiles_enabled_flag_offset")] = true;
        data_.insert(data_.begin() + metadata_.GetValue("tile_dimensions_offset"), make_move_iterator(dimensions_bits.begin()), make_move_iterator(dimensions_bits.end()));
        data_.ByteAlignWithoutRemoval();

        tile_dimensions_ = dimensions;
    }
}; //namespace lightdb::hevc

