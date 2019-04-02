#include "Nal.h"
#include "Emulation.h"
#include "SequenceParameterSet.h"

namespace lightdb::hevc {

    SequenceParameterSet::SequenceParameterSet(const StitchContext &context, const bytestring &data)
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

        dimensions_.first = metadata_.GetValue("height");
        dimensions_.second = metadata_.GetValue("width");
        log2_max_pic_order_cnt_lsb_ = metadata_.GetValue("log2_max_pic_order_cnt_lsb_minus4") + 4;
        CalculateSizes();
    }

    void SequenceParameterSet::SetDimensions(const std::pair<unsigned int, unsigned int> &dimensions) {
        assert(dimensions.first > 0 && dimensions.second > 0);

        // The dimensions are width first, then height, so we must reverse the
        // order from our height, width
        std::vector<unsigned long> new_dimensions(kNumDimensions);
        new_dimensions[1] = static_cast<unsigned long>(dimensions.first);
        new_dimensions[0] = static_cast<unsigned long>(dimensions.second);

        std::vector<unsigned long> old_dimensions(kNumDimensions);
        old_dimensions[0] = dimensions_.second;
        old_dimensions[1] = dimensions_.first;

        auto new_dimension_bits = EncodeGolombs(new_dimensions);
        auto old_dimension_bits = EncodeGolombs(old_dimensions);

        auto start = metadata_.GetValue("dimensions_offset");

        // Replace the old dimensions with the new, combining the portion before the old dimensions, the new
        // dimensions, and the portion after the old dimensions
        data_.Replace(start, start + old_dimension_bits.size(), new_dimension_bits);
        data_.ByteAlign();

        dimensions_.first = static_cast<unsigned long>(dimensions.first);
        dimensions_.second = static_cast<unsigned long>(dimensions.second);
        CalculateSizes();
    }

    void SequenceParameterSet::CalculateSizes() {
        auto min_cb_log2_size = metadata_.GetValue("log2_min_luma_coding_block_size_minus3") + 3;

        assert ((GetContext().GetVideoDimensions().first & (1 << min_cb_log2_size)) == 0 &&
                (GetContext().GetVideoDimensions().second & (1 << min_cb_log2_size)) == 0);

        auto min_cb_log2_size_y = min_cb_log2_size;
        auto ctb_log2_size_y = min_cb_log2_size_y + metadata_.GetValue("log2_diff_max_min_luma_coding_block_size");
        auto ctb_size_y = 1 << ctb_log2_size_y;
        auto pic_width_in_ctbs_y = static_cast<size_t>(ceil(dimensions_.second / static_cast<double>(ctb_size_y)));
        auto pic_height_in_ctbs_y = static_cast<size_t>(ceil(dimensions_.first / static_cast<double>(ctb_size_y)));
        auto pic_size_in_ctbs_y = pic_width_in_ctbs_y * pic_height_in_ctbs_y;

        address_length_in_bits_ = static_cast<size_t>(ceil(log2(pic_size_in_ctbs_y)));
        assert (address_length_in_bits_ > 0);

        size_t ctu_dimensions[2];
        ctu_dimensions[0] = pic_height_in_ctbs_y;
        ctu_dimensions[1] = pic_width_in_ctbs_y;

        auto delta_x = ctu_dimensions[1] / GetContext().GetTileDimensions().second;
        auto delta_y = ((delta_x * (ctu_dimensions[0] / GetContext().GetTileDimensions().first)) *
                GetContext().GetTileDimensions().second);

        addresses_.clear();
        auto tile_dimensions = GetContext().GetTileDimensions();
        auto row = 0u;
        while (row < tile_dimensions.first) {
            auto col = 0u;
            while (col < tile_dimensions.second) {
                addresses_.push_back(delta_x * (col % tile_dimensions.second) + delta_y * row);
                col++;
            }
            row++;
        }
        assert(addresses_.size() == tile_dimensions.first * tile_dimensions.second);
    }
}; //namespace lightdb::hevc

