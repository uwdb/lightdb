#ifndef LIGHTDB_SEQUENCEPARAMETERSET_H
#define LIGHTDB_SEQUENCEPARAMETERSET_H

#include "Nal.h"
#include "BitStream.h"
#include "BitArray.h"
#include "Context.h"
#include <vector>


namespace lightdb {

    // Described in 7.3.2.2

    //TODO: Maybe add a getter for general_level_idc, also add assertions mb in calculate_sizes

    class SequenceParameterSet : public Nal {
    public:
        /**
         * Interprets data as a byte stream representing a SequenceParameterSet
         * @param context The context surrounding the Nal
         * @param data The byte stream
         */
        SequenceParameterSet(const Context &context, const bytestring &data);

        /**
         * Sets the tile dimensions in the byte stream to be dimensions, unit of measurement being luma samples
         * @param dimensions A length two array, the first element being the height and the second
         * being the width
         */
        void SetDimensions(const std::pair<unsigned int, unsigned int> dimensions);

        /**
         *
         * @return An array representing the tile dimensions, height first, then width.
         * Note that changing this array changes this header's tile dimensions
         */
        const std::pair<unsigned long, unsigned long> GetTileDimensions() const;

        /**
         * Sets the general level IDC value in the byte stream to be value, converting value to a byte
         * @param value The new general level IDC value
         */
        void SetGeneralLevelIDC(const unsigned int value);

        /**
         *
         * @return The address length in bits
         */
        size_t GetAddressLength() const;

        /**
         *
         * @return The log2_max_pic_order_cnt_lsb_ for this Nal
         */
        unsigned long GetMaxPicOrder() const;

        /**
         *
         * @return A string with the bytes of this Nal
         */
        bytestring GetBytes() const override;

        /**
         * @return A vector containing the tile addresses
         */
         const std::vector<size_t>& GetAddresses() const;

    private:

        void CalculateSizes();

        utility::BitArray data_;
        utility::BitStream metadata_;
        size_t address_length_in_bits_;
        std::vector<size_t> addresses_;
        std::pair<unsigned long, unsigned long> dimensions_;
        unsigned long log2_max_pic_order_cnt_lsb_;

        static const int kSizeBeforeProfile = 1;
        static const int kGeneralLevelIDCSize = 1;
        static const int kNumDimensions = 2;

    };
}

#endif //LIGHTDB_SEQUENCEPARAMETERSET_H
