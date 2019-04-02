#ifndef LIGHTDB_SEQUENCEPARAMETERSET_H
#define LIGHTDB_SEQUENCEPARAMETERSET_H

#include "Nal.h"
#include "BitStream.h"
#include "BitArray.h"
#include "StitchContext.h"
#include "Profile.h"
#include "Emulation.h"
#include <vector>


namespace lightdb::hevc {

    // Described in 7.3.2.2

    //TODO: Maybe add a getter for general_level_idc, also add assertions mb in calculate_sizes

    class SequenceParameterSet : public Nal {
    public:
        /**
         * Interprets data as a byte stream representing a SequenceParameterSet
         * @param context The context surrounding the Nal
         * @param data The byte stream
         */
        SequenceParameterSet(const StitchContext &context, const bytestring &data);

        /**
         * Sets the tile dimensions in the byte stream to be dimensions, unit of measurement being luma samples
         * @param dimensions A length two array, the first element being the height and the second
         * being the width
         */
        void SetDimensions(const std::pair<unsigned int, unsigned int> &dimensions);

        /**
         *
         * @return An array representing the tile dimensions, height first, then width.
         * Note that changing this array changes this header's tile dimensions
         */
        inline const std::pair<unsigned long, unsigned long>& GetTileDimensions() const {
            return dimensions_;
        }

        /**
         * Sets the general level IDC value in the byte stream to be value, converting value to a byte
         * @param value The new general level IDC value
         */
        inline void SetGeneralLevelIDC(const unsigned int value) {
            auto profile_size = GetSizeInBits(metadata_.GetValue("sps_max_sub_layer_minus1"));
            assert (profile_size % 8 == 0);
            data_.SetByte(GetHeaderSize() + kSizeBeforeProfile + profile_size / 8 - kGeneralLevelIDCSize, static_cast<unsigned char>(value));
        }

        /**
         *
         * @return The address length in bits
         */
        inline size_t GetAddressLength() const {
            return address_length_in_bits_;
        }

        /**
         *
         * @return The log2_max_pic_order_cnt_lsb_ for this Nal
         */
        inline unsigned long GetMaxPicOrder() const {
            return log2_max_pic_order_cnt_lsb_;
        }

        /**
         *
         * @return A string with the bytes of this Nal
         */
        inline bytestring GetBytes() const override {
            return AddEmulationPreventionAndMarker(data_, GetHeaderSize(), data_.size() / 8);
        }

        /**
         * @return A vector containing the tile addresses
         */
         inline const std::vector<size_t>& GetAddresses() const {
            return addresses_;
        }

    private:

        void CalculateSizes();

        BitArray data_;
        BitStream metadata_;
        size_t address_length_in_bits_;
        std::vector<size_t> addresses_;
        std::pair<unsigned long, unsigned long> dimensions_;
        unsigned long log2_max_pic_order_cnt_lsb_;

        static constexpr unsigned int kSizeBeforeProfile = 1;
        static constexpr unsigned int kGeneralLevelIDCSize = 1;
        static constexpr unsigned int kNumDimensions = 2;

    };
}; //namespace lightdb::hevc

#endif //LIGHTDB_SEQUENCEPARAMETERSET_H
