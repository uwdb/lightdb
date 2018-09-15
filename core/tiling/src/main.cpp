#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <gtest/gtest.h>

#include "Context.h"
#include "Stitcher.h"

using std::string;
using std::vector;
using lightdb::Context;
using lightdb::Stitcher;

int main(int argc, char *argv[]) {
    int num_tiles = std::stoi(argv[1]);
    vector<bytestring> tiles(num_tiles);
    for (int i = 0; i < num_tiles; i++) {
        std::ifstream istrm;
        istrm.open(argv[i + 2]);
        std::stringstream buffer;
        buffer << istrm.rdbuf();
        string tile = buffer.str();
        tiles[i] = vector<char>(tile.begin(), tile.end());
    }

    int tile_dimensions[2];
    int video_dimensions[2];
    tile_dimensions[0] = std::stoi(argv[num_tiles + 2]);
    tile_dimensions[1] = std::stoi(argv[num_tiles + 3]);
    video_dimensions[0] = std::stoi(argv[num_tiles + 4]);
    video_dimensions[1] = std::stoi(argv[num_tiles + 5]);

    Context context(tile_dimensions, video_dimensions);
    Stitcher stitcher(context, tiles);
    bytestring stitched = stitcher.GetStitchedSegments();

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