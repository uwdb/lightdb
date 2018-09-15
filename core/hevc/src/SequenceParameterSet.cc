#include "Nal.h"
#include "Emulation.h"
#include "SequenceParameterSet.h"
#include "Profile.h"
#include "Golombs.h"

namespace lightdb {

    SequenceParameterSet::SequenceParameterSet(const Context &context, const bytestring &data)
            : Nal(context, data),
              data_(RemoveEmulationPrevention(data, GetHeaderSize(), data.size())),
              metadata_(data_.begin(), data_.begin() + GetHeaderSizeInBits()) {

        metadata_.SkipBits(4);
        metadata_.CollectValue("sps_max_sub_layer_minus1", 3);
        metadata_.SkipBits(1);
        metadata_.SkipBits(GetSizeInBits(metadata_.GetValue("sps_max_sub_layer_minus1")));
        metadata_.SkipExponentialGolomb();
        metadata_.CollectGolomb("chroma_format_idc");

        auto skip_bits = 0u;
        if (metadata_.GetValue("chroma_format_idc") == 3) {
            skip_bits = 1;
        }

        metadata_.SkipBits(skip_bits);
        metadata_.MarkPosition("dimensions_offset");
        metadata_.CollectGolomb("width");
        metadata_.CollectGolomb("height");
        metadata_.CollectValue("conformance_window_flag");
        metadata_.SkipExponentialGolombs("conformance_window_flag", 4);
        metadata_.SkipExponentialGolomb();
        metadata_.SkipExponentialGolomb();
        metadata_.CollectGolomb("log2_max_pic_order_cnt_lsb_minus4");
        metadata_.CollectValue("sps_sub_layer_ordering_info_present_flag");

        auto skip_golombs = metadata_.GetValue("sps_max_sub_layer_minus1");
        if (metadata_.GetValue("sps_sub_layer_ordering_info_present_flag")) {
            skip_golombs = 3 * (skip_golombs + 1);
        } else {
            skip_golombs *= 3;
        }

        metadata_.SkipExponentialGolombs(skip_golombs);
        metadata_.CollectGolomb("log2_min_luma_coding_block_size_minus3");
        metadata_.CollectGolomb("log2_diff_max_min_luma_coding_block_size");

        dimensions_[0] = metadata_.GetValue("height");
        dimensions_[1] = metadata_.GetValue("width");
        log2_max_pic_order_cnt_lsb_ = metadata_.GetValue("log2_max_pic_order_cnt_lsb_minus4") + 4;
        CalculateSizes();
    }

    bytestring SequenceParameterSet::GetBytes() const {
        return AddEmulationPreventionAndMarker(data_, GetHeaderSize(), data_.size() / 8);
    }

    unsigned long SequenceParameterSet::GetMaxPicOrder() const {
        return log2_max_pic_order_cnt_lsb_;
    }

    void SequenceParameterSet::SetDimensions(const unsigned int *dimensions) {
        assert(dimensions[0] > 0 && dimensions[1] > 0);

        // The dimensions are width first, then height, so we must reverse the
        // order from our height, width
        std::vector<unsigned long> new_dimensions(kNumDimensions);
        new_dimensions[1] = static_cast<unsigned long>(dimensions[0]);
        new_dimensions[0] = static_cast<unsigned long>(dimensions[1]);

        std::vector<unsigned long> old_dimensions(kNumDimensions);
        old_dimensions[0] = dimensions_[1];
        old_dimensions[1] = dimensions_[0];

        auto new_dimension_bits = EncodeGolombs(new_dimensions);
        auto old_dimension_bits = EncodeGolombs(old_dimensions);

        auto start = metadata_.GetValue("dimensions_offset");

        // Replace the old dimensions with the new, combining the portion before the old dimensions, the new
        // dimensions, and the portion after the old dimensions
        data_.Replace(start, start + old_dimension_bits.size(), new_dimension_bits);
        data_.ByteAlign();

        dimensions_[0] = static_cast<unsigned long>(dimensions[0]);
        dimensions_[1] = static_cast<unsigned long>(dimensions[1]);
        CalculateSizes();
    }

    void SequenceParameterSet::SetGeneralLevelIDC(const unsigned int value) {
        auto profile_size = GetSizeInBits(metadata_.GetValue("sps_max_sub_layer_minus1"));
        assert (profile_size % 8 == 0);
        data_.SetByte(GetHeaderSize() + kSizeBeforeProfile + profile_size / 8 - kGeneralLevelIDCSize, static_cast<unsigned char>(value));
    }

    size_t SequenceParameterSet::GetAddressLength() const {
        return address_length_in_bits_;
    }

    const unsigned long * SequenceParameterSet::GetTileDimensions() const {
        return dimensions_;
    }

    const std::vector<size_t>& SequenceParameterSet::GetAddresses() const {
        return addresses_;
    }

    void SequenceParameterSet::CalculateSizes() {
        auto min_cb_log2_size = metadata_.GetValue("log2_min_luma_coding_block_size_minus3") + 3;

        assert ((GetContext().GetVideoDimensions()[0] & (1 << min_cb_log2_size)) == 0 &&
                (GetContext().GetVideoDimensions()[1] & (1 << min_cb_log2_size)) == 0);

        auto min_cb_log2_size_y = min_cb_log2_size;
        auto ctb_log2_size_y = min_cb_log2_size_y + metadata_.GetValue("log2_diff_max_min_luma_coding_block_size");
        auto ctb_size_y = 1 << ctb_log2_size_y;
        auto pic_width_in_ctbs_y = static_cast<size_t>(ceil(dimensions_[1] / static_cast<double>(ctb_size_y)));
        auto pic_height_in_ctbs_y = static_cast<size_t>(ceil(dimensions_[0] / static_cast<double>(ctb_size_y)));
        auto pic_size_in_ctbs_y = pic_width_in_ctbs_y * pic_height_in_ctbs_y;

        address_length_in_bits_ = static_cast<size_t>(ceil(log2(pic_size_in_ctbs_y)));
        assert (address_length_in_bits_ > 0);

        size_t ctu_dimensions[2];
        ctu_dimensions[0] = pic_height_in_ctbs_y;
        ctu_dimensions[1] = pic_width_in_ctbs_y;

        auto delta_x = ctu_dimensions[1] / GetContext().GetTileDimensions()[1];
        auto delta_y = ((delta_x * (ctu_dimensions[0] / GetContext().GetTileDimensions()[0])) *
                GetContext().GetTileDimensions()[1]);

        addresses_.clear();
        auto tile_dimensions = GetContext().GetTileDimensions();
        auto row = 0u;
        while (row < tile_dimensions[0]) {
            auto col = 0u;
            while (col < tile_dimensions[1]) {
                addresses_.push_back(delta_x * (col % tile_dimensions[1]) + delta_y * row);
                col++;
            }
            row++;
        }
        assert(addresses_.size() == tile_dimensions[0] * tile_dimensions[1]);
    }
}

