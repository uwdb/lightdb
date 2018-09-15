#include "PictureParameterSet.h"
#include "Emulation.h"
#include "Golombs.h"

namespace lightdb {

    PictureParameterSet::PictureParameterSet(const Context &context, const bytestring &data)
            : Nal(context, data),
              data_(RemoveEmulationPrevention(data, GetHeaderSize(), data.size())),
              metadata_(data_.begin(), data_.begin() + GetHeaderSizeInBits()) {
        metadata_.SkipExponentialGolomb();
        metadata_.SkipExponentialGolomb();
        metadata_.SkipBits(6);
        metadata_.CollectValue("cabac_init_present_flag");
        metadata_.SkipExponentialGolomb();
        metadata_.SkipExponentialGolomb();
        metadata_.SkipExponentialGolomb();
        metadata_.SkipBits(2);
        metadata_.CollectValue("cu_qp_delta_enabled_flag");
        metadata_.SkipExponentialGolombs("cu_qp_delta_enabled_flag", 1);
        metadata_.SkipExponentialGolomb();
        metadata_.SkipExponentialGolomb();
        metadata_.SkipBits(4);
        metadata_.MarkPosition("tiles_enabled_flag_offset");
        metadata_.CollectValue("tiles_enabled_flag");
        metadata_.CollectValue("entropy_coding_sync_enabled_flag");
        metadata_.MarkPosition("tile_dimensions_offset");
        metadata_.ByteAlign();
        metadata_.MarkPosition("end");

        CHECK_EQ(metadata_.GetValue("end") % 8, 0);
    }

    bytestring PictureParameterSet::GetBytes() const {
        return AddEmulationPreventionAndMarker(data_, GetHeaderSize(), data_.size() / 8);
    }

    bool PictureParameterSet::HasEntryPointOffsets() const {
        return metadata_.GetValue("tiles_enabled_flag") ||
               metadata_.GetValue("entropy_coding_sync_enabled_flag");
    }

    bool PictureParameterSet::CabacInitPresentFlag() const {
        return static_cast<bool>(metadata_.GetValue("cabac_init_present_flag"));
    }

    void PictureParameterSet::SetTileDimensions(const std::pair<unsigned int, unsigned int>& dimensions, const bool loop_filter_enabled) {
        if (dimensions.first == tile_dimensions_[0] &&
            dimensions.second == tile_dimensions_[1]) {
            return;
        }

        assert(dimensions.first > 0 && dimensions.second > 0);
        assert(!metadata_.GetValue("tiles_enabled_flag"));
        assert(tile_dimensions_[0] == 0 && tile_dimensions_[1] == 0);

        // The dimensions are width first, then height, so we must reverse the
        // order from our height, width
        // Next comes a one, and then a 1 if the filter is enabled, 0 if not
        std::vector<unsigned long> new_dimensions(2);
        new_dimensions[0] = static_cast<unsigned long>(dimensions.second - 1);
        new_dimensions[1] = static_cast<unsigned long>(dimensions.first - 1);

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

        tile_dimensions_[0] = dimensions.first;
        tile_dimensions_[1] = dimensions.second;
    }

    const unsigned int *PictureParameterSet::GetTileDimensions() {
        return tile_dimensions_;
    }
}

