#ifndef LIGHTDB_CONTEXT_H
#define LIGHTDB_CONTEXT_H

#include <utility>

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
        Context(const std::pair<unsigned int, unsigned int>& tile_dimensions,
                const std::pair<unsigned int, unsigned int>& video_dimensions)
                : tile_dimensions_{tile_dimensions},
                  video_dimensions_{tile_dimensions.first * video_dimensions.first, tile_dimensions.second * video_dimensions.second}
        { }

        /**
         *
         * @return The tile dimensions. Note that this gives the user the ability to change the dimensions
         */
        // TODO change to using dimensions=std::pair
        inline const std::pair<unsigned int, unsigned int> GetTileDimensions() {
            return tile_dimensions_;
        }
        /**
         *
         * @return The video dimensions. Note that this gives the user the ability to change the dimensions
         */
        inline const std::pair<unsigned int, unsigned int>& GetVideoDimensions() {
            return video_dimensions_;
        }

    private:

        std::pair<unsigned int, unsigned int> tile_dimensions_;
        std::pair<unsigned int, unsigned int> video_dimensions_;

    };

}

#endif //LIGHTDB_CONTEXT_H
