//
// Created by sophi on 4/9/2018.
//

#ifndef LIGHTDB_STITCHER_H
#define LIGHTDB_STITCHER_H

#include <string>
#include <vector>
#include "Headers.h"
#include "Context.h"

namespace lightdb {
    class Stitcher {
     public:

        /**
         * Creates a Stitcher object, which involves splitting all of the tiles into their component nals. It also moves all of the data from
         * the tiles passed in, rendering "data" useless after the constructor
         * @param context The context of the video data
         * @param data A vector with each element being the bytestring of a tile. All data is moved from this vector, rendering it useless post
         * processing
         */
        Stitcher(const Context &context, std::vector<bytestring> &data) : tiles_(data), context_(context), headers_(context_, GetNals().front())  {
        }

        /**
         *
         * @return A bytestring with all tiles passed into the constructor stitched together
         */
        bytestring GetStitchedSegments();


     private:

        /**
         *
         * @return The tile_nals_ field populated with the nals of each tile. Each element of the outer vector is a tile, and each element of the inner
         * vector is a nal for tha tile
         */
        std::vector<std::vector<bytestring>> GetNals();

        /**
         * Returns the nals that are segments for a given tile. Note that this moves the corresponding nal from the tile_nals_ field, rendering the
         * index for each affected nal useless in the field
         * @param tile_num The index of the tile in the tile_nals_ vector
         * @param num_bytes A running count of the number of bytes the segment nals of all the tiles occupy. This is incremented by the number of bytes
         * the segments of this tile_num occupy
         * @param num_keyframes A running count of the number of keyframes the tiles in this stitch have. This is incremented if "first" is true (to avoid
         * incrementing the count for each tile, since we only need to count the number in one tile to know the number for the final stitch)
         * @param first Whether or not this is the first tile being processed
         * @return The nals that are segments for this tile
         */
        std::vector<bytestring> GetSegmentNals(const unsigned long tile_num, unsigned long *num_bytes, unsigned long *num_keyframes, bool first);

        std::vector<bytestring> tiles_;
        std::vector<std::vector<bytestring>> tile_nals_;
        Context context_;
        Headers headers_;
    };

}

#endif //LIGHTDB_STITCHER_H
