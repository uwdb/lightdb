#include "Context.h"
#include "Stitcher.h"
#include "Encoding.h"
#include <gtest/gtest.h>

using std::string;
using std::vector;
using lightdb::Context;
using lightdb::Stitcher;

int main(int argc, char *argv[]) {
    auto num_tiles = std::stoul(argv[1]);
    vector<lightdb::bytestring> tiles(num_tiles);
    for (auto i = 0ul; i < num_tiles; i++) {
        std::ifstream istrm;
        istrm.open(argv[i + 2]);
        std::stringstream buffer;
        buffer << istrm.rdbuf();
        string tile = buffer.str();
        tiles[i] = vector<char>(tile.begin(), tile.end());
    }

    unsigned int tile_dimensions[2];
    unsigned int video_dimensions[2];
    tile_dimensions[0] = static_cast<unsigned int>(std::stoul(argv[num_tiles + 2]));
    tile_dimensions[1] = static_cast<unsigned int>(std::stoul(argv[num_tiles + 3]));
    video_dimensions[0] = static_cast<unsigned int>(std::stoul(argv[num_tiles + 4]));
    video_dimensions[1] = static_cast<unsigned int>(std::stoul(argv[num_tiles + 5]));

    Context context(tile_dimensions, video_dimensions);
    Stitcher stitcher(context, tiles);
    lightdb::bytestring stitched = stitcher.GetStitchedSegments();

    std::ofstream ostrm;
    ostrm.open(argv[num_tiles + 6]);
    for (auto c : stitched) {
        ostrm << c;
    }
    ostrm.close();

    return 0;

    /**
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS(); **/
}