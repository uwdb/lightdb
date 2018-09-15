//
// Created by sophi on 5/3/2018.
//

#include "Context.h"

namespace lightdb {


    Context::Context(const unsigned int *tile_dimensions, const unsigned int *video_dimensions) {
        tile_dimensions_[0] = tile_dimensions[0];
        tile_dimensions_[1] = tile_dimensions[1];

        video_dimensions_[0] = video_dimensions[0] * tile_dimensions[0];
        video_dimensions_[1] = video_dimensions[1] * tile_dimensions[1];
    }

    unsigned int* Context::GetTileDimensions() {
        return tile_dimensions_;
    }

    unsigned int* Context::GetVideoDimensions() {
        return video_dimensions_;
    }

}
