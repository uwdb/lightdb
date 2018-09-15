#include "Context.h"
#include "Stitcher.h"
#include <gtest/gtest.h>

class StitcherTestFixture : public testing::Test {
};

TEST_F(StitcherTestFixture, testStitch) {
    const std::vector<std::string> filenames{"tile0.hevc", "tile1.hevc", "tile2.hevc", "tile3.hevc"};
    const std::string output_filename{"out.hevc"};
    std::vector<lightdb::bytestring> tiles(filenames.size());

    for (auto i = 0ul; i < filenames.size(); i++) {
        std::ifstream istrm;
        istrm.open(filenames[i]);
        std::stringstream buffer;
        buffer << istrm.rdbuf();
        std::string tile = buffer.str();
        tiles[i] = std::vector<char>(tile.begin(), tile.end());
        ASSERT_GT(tiles[i].size(), 0);
    }

    std::pair<unsigned int, unsigned int> tile_dimensions(2, 2);
    std::pair<unsigned int, unsigned int> video_dimensions(1920, 1080);

    lightdb::Context context(tile_dimensions, video_dimensions);
    lightdb::Stitcher stitcher(context, tiles);
    lightdb::bytestring stitched = stitcher.GetStitchedSegments();

    std::ofstream ostrm;
    ostrm.open(output_filename);
    for (auto c : stitched) {
        ostrm << c;
    }
    ostrm.close();
}