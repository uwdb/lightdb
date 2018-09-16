#include "Context.h"
#include "Stitcher.h"
#include "AssertVideo.h"
#include "functional.h"
#include <gtest/gtest.h>

using lightdb::bytestring;

class StitcherTestFixture : public testing::Test {
protected:
    static constexpr unsigned int frames = 600;
    static constexpr char const * kOutputFilename = "out.hevc";

    static bytestring load_tile(const size_t index) {
        return load_tile(tile_filename(index));
    }

private:
    static std::string tile_filename(const size_t index) {
        return std::string("resources/tiles/tile-" + std::to_string(index) + ".hevc");
    }

    static bytestring load_tile(const std::string &filename) {
        std::ifstream stream{filename};
        bytestring data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        EXPECT_GT(data.size(), 0);
        return data;
    }
};

TEST_F(StitcherTestFixture, testStitch2x1) {
    auto rows = 2u, columns = 1u;
    auto height = 512u, width = 960u;
    auto indexes = { 0u,
                     4u};
    auto tiles = lightdb::functional::transform<bytestring>(
            indexes.begin(), indexes.end(),
            [](auto i) { return load_tile(i); });

    {
        lightdb::hevc::Context context({rows, columns}, {height, width});
        lightdb::hevc::Stitcher stitcher(context, tiles);
        std::ofstream out(kOutputFilename);

        for (auto chunk : stitcher.GetStitchedSegments())
            out << chunk;
    }

    EXPECT_VIDEO_VALID(kOutputFilename);
    EXPECT_VIDEO_FRAMES(kOutputFilename, frames);
    EXPECT_VIDEO_RESOLUTION(kOutputFilename, rows*height - 4, columns*width);
    EXPECT_EQ(remove(kOutputFilename), 0);
}

TEST_F(StitcherTestFixture, testStitch1x2) {
    auto rows = 1u, columns = 2u;
    auto height = 512u, width = 960u;
    auto indexes = { 0u, 1u };
    auto tiles = lightdb::functional::transform<bytestring>(
            indexes.begin(), indexes.end(),
            [](auto i) { return load_tile(i); });

    {
        lightdb::hevc::Context context({rows, columns}, {height, width});
        lightdb::hevc::Stitcher stitcher(context, tiles);
        std::ofstream out(kOutputFilename);

        for (auto chunk : stitcher.GetStitchedSegments())
            out << chunk;
    }

    EXPECT_VIDEO_VALID(kOutputFilename);
    EXPECT_VIDEO_FRAMES(kOutputFilename, frames);
    EXPECT_VIDEO_RESOLUTION(kOutputFilename, rows*height - 4, columns*width);
    EXPECT_EQ(remove(kOutputFilename), 0);
}

TEST_F(StitcherTestFixture, testStitch2x2) {
    auto rows = 2u, columns = 2u;
    auto height = 512u, width = 960u;
    auto indexes = { 0u,  1u,
                     4u,  5u};
    auto tiles = lightdb::functional::transform<bytestring>(
            indexes.begin(), indexes.end(),
            [](auto i) { return load_tile(i); });

    {
        lightdb::hevc::Context context({rows, columns}, {height, width});
        lightdb::hevc::Stitcher stitcher(context, tiles);
        std::ofstream out(kOutputFilename);

        for (auto chunk : stitcher.GetStitchedSegments())
            out << chunk;
    }

    EXPECT_VIDEO_VALID(kOutputFilename);
    EXPECT_VIDEO_FRAMES(kOutputFilename, frames);
    EXPECT_VIDEO_RESOLUTION(kOutputFilename, rows*height - 4, columns*width);
    EXPECT_EQ(remove(kOutputFilename), 0);
}

TEST_F(StitcherTestFixture, testStitch4x4) {
    auto rows = 4u, columns = 4u;
    auto height = 512u, width = 960u;
    auto indexes = { 0u,  1u,  2u,  3u,
                     4u,  5u,  6u,  7u,
                     8u,  9u, 10u, 11u,
                    12u, 13u, 14u, 15u};
    auto tiles = lightdb::functional::transform<bytestring>(
            indexes.begin(), indexes.end(),
            [](auto i) { return load_tile(i); });

   {
       lightdb::hevc::Context context({rows, columns}, {height, width});
       lightdb::hevc::Stitcher stitcher(context, tiles);
       std::ofstream out(kOutputFilename);

       for (auto chunk : stitcher.GetStitchedSegments())
           out << chunk;
    }

    EXPECT_VIDEO_VALID(kOutputFilename);
    EXPECT_VIDEO_FRAMES(kOutputFilename, frames);
    EXPECT_VIDEO_RESOLUTION(kOutputFilename, rows*height - 4, columns*width);
    EXPECT_EQ(remove(kOutputFilename), 0);
}