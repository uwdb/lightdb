#ifndef LIGHTDB_PICTUREPARAMETERSET_H
#define LIGHTDB_PICTUREPARAMETERSET_H

#include "Nal.h"
#include <string>
#include <vector>

//TODO: Fix that you get direct access to dimensions?

namespace lightdb {
    // Defined in 7.3.2.3
    class PictureParameterSet : public Nal {
    public:
        // Should the char * be const?

        /**
         * Interprets data as a byte stream representing a PictureParameterSet
         * @param context The context surrounding the Nal
         * @param data The byte stream
         */
        PictureParameterSet(const Context &context, const bytestring &data);

        /**
         * Sets the tile dimensions in the byte stream to be dimensions
         * @param dimensions A length two array, the first element being the height (num of rows) and the second
         * being the width (num of columns)
         * @param loop_filter_enabled Set to false unless otherwise specified
         */
        void SetTileDimensions(const unsigned int *dimensions, const bool loop_filter_enabled = false);

        /**
         *
         * @return An array representing the tile dimensions, height first, then width.
         * Note that changing this array changes this header's tile dimensions
         */
        unsigned int *GetTileDimensions();

        /**
         *
         * @return True if this byte stream has entry point offsets, false otherwise
         */
        bool HasEntryPointOffsets() const;

        /**
         *
         * @return True if the cabac_init_present_flag is set to true in this byte stream.
         * false otherwise
         */
	    bool CabacInitPresentFlag() const;

        /**
         *
         * @return A string wtih the bytes of this Nal
         */
        bytestring GetBytes() const override;

    private:
        utility::BitArray data_;
        utility::BitStream metadata_;
        unsigned int tile_dimensions_[2];
    };
}

#endif //LIGHTDB_PICTUREPARAMETERSET_H
