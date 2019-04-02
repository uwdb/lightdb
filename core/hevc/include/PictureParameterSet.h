#ifndef LIGHTDB_PICTUREPARAMETERSET_H
#define LIGHTDB_PICTUREPARAMETERSET_H

#include "Nal.h"
#include "BitArray.h"
#include "BitStream.h"
#include "Emulation.h"

namespace lightdb::hevc {
    // Defined in 7.3.2.3
    class PictureParameterSet : public Nal {
    public:
        /**
         * Interprets data as a byte stream representing a PictureParameterSet
         * @param context The context surrounding the Nal
         * @param data The byte stream
         */
        PictureParameterSet(const StitchContext &context, const bytestring &data)
                : Nal(context, data),
                  data_(RemoveEmulationPrevention(data, GetHeaderSize(), data.size())),
                  metadata_(data_.begin(), data_.begin() + GetHeaderSizeInBits()),
                  tile_dimensions_{0, 0} {
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

        /**
         * Sets the tile dimensions in the byte stream to be dimensions
         * @param dimensions A length two array, the first element being the height (num of rows) and the second
         * being the width (num of columns)
         * @param loop_filter_enabled Set to false unless otherwise specified
         */
        void SetTileDimensions(const std::pair<unsigned long, unsigned long>& dimensions,
                               bool loop_filter_enabled = false);

        /**
         *
         * @return An array representing the tile dimensions, height first, then width.
         * Note that changing this array changes this header's tile dimensions
         */
        inline const std::pair<unsigned long, unsigned long>& GetTileDimensions() const {
            return tile_dimensions_;
        }

        /**
         *
         * @return True if this byte stream has entry point offsets, false otherwise
         */
        inline bool HasEntryPointOffsets() const {
            return metadata_.GetValue("tiles_enabled_flag") ||
                   metadata_.GetValue("entropy_coding_sync_enabled_flag");
        }

        /**
         *
         * @return True if the cabac_init_present_flag is set to true in this byte stream.
         * false otherwise
         */
	    inline bool CabacInitPresentFlag() const {
            return static_cast<bool>(metadata_.GetValue("cabac_init_present_flag"));
        }

        /**
         *
         * @return A string wtih the bytes of this Nal
         */
        inline bytestring GetBytes() const override {
            return AddEmulationPreventionAndMarker(data_, GetHeaderSize(), data_.size() / 8);
        }

    private:
        BitArray data_;
        BitStream metadata_;
        std::pair<unsigned long, unsigned long> tile_dimensions_;
    };
}; //namespace lightdb::hevc

#endif //LIGHTDB_PICTUREPARAMETERSET_H
