#ifndef LIGHTDB_CONTEXT_H
#define LIGHTDB_CONTEXT_H

namespace lightdb {

    class Context {
    public:

        /**
         * Constructs a context with the given tile and video dimensions
         * @param tile_dimensions The tile dimensions, first element being the height (number of rows)
         * and the second being the width (number of columns)
         * @param video_dimensions  The video dimensions, first element being the height and the second
         * the width
         */
        Context(const unsigned int *tile_dimensions, const unsigned int *video_dimensions);
        /**
         *
         * @return The tile dimensions. Note that this gives the user the ability to change the dimensions
         */
        // TODO change to std::pair
        const unsigned int* GetTileDimensions();
        /**
         *
         * @return The video dimensions. Note that this gives the user the ability to change the dimensions
         */
        const unsigned int* GetVideoDimensions();

    private:

        // TODO change to std::pair
        unsigned int tile_dimensions_[2];
        unsigned int video_dimensions_[2];

    };

}

#endif //LIGHTDB_CONTEXT_H
