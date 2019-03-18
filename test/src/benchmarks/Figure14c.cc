#include "HeuristicOptimizer.h"
#include "Ffmpeg.h"
#include "TestResources.h"
#include "AssertVideo.h"
#include "AssertTime.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;
using namespace lightdb::video::ffmpeg;

class Figure14cTestFixture : public testing::Test {
public:
    Figure14cTestFixture()
            : catalog(Resources.catalog_name),
              path(std::filesystem::absolute(LIGHTDB_BENCHMARK_DATASET_PATH)) {
        Catalog::instance(catalog);
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
    }

    void testDegenerateTimeSelect(const std::string &dataset) {
        auto filename = path / dataset;
        auto configuration = GetStreamConfiguration(filename, 0, true);
        auto frames = COUNT_FRAMES(filename);

        auto query = Load(filename)
                         .Select(TemporalRange::limits())
                         .Encode()
                         .Save(Resources.out.h264);

        LOG_DURATION(dataset,
            ASSERT_MSECS(
                Coordinator().execute(query),
                400u));

        EXPECT_VIDEO_VALID(Resources.out.h264);
        EXPECT_VIDEO_FRAMES(Resources.out.h264, frames);
        EXPECT_VIDEO_RESOLUTION(Resources.out.h264, configuration.height, configuration.width);
        EXPECT_EQ(remove(Resources.out.h264), 0);
    }

    void testDegenerateAngularSelect(const std::string &dataset) {
        auto filename = path / dataset;
        auto configuration = GetStreamConfiguration(filename, 0, true);
        auto frames = COUNT_FRAMES(filename);

        auto query = Load(filename)
                         .Select(ThetaRange::limits())
                         .Encode()
                         .Save(Resources.out.h264);

        LOG_DURATION(dataset,
                     ASSERT_MSECS(
                             Coordinator().execute(query),
                             400u));

        EXPECT_VIDEO_VALID(Resources.out.h264);
        EXPECT_VIDEO_FRAMES(Resources.out.h264, frames);
        EXPECT_VIDEO_RESOLUTION(Resources.out.h264, configuration.height, configuration.width);
        EXPECT_EQ(remove(Resources.out.h264), 0);
    }

protected:
    Catalog catalog;
    const std::filesystem::path path;
};

TEST_F(Figure14cTestFixture, testSelectTime_1K) {
    testDegenerateTimeSelect("timelapse/timelapse1K.h264");
}

TEST_F(Figure14cTestFixture, testSelectTime_2K) {
    testDegenerateTimeSelect("timelapse/timelapse2K.h264");
}

TEST_F(Figure14cTestFixture, testSelectTime_4K) {
    testDegenerateTimeSelect("timelapse/timelapse4K.h264");
}

TEST_F(Figure14cTestFixture, testSelectAngle_1K) {
    testDegenerateAngularSelect("timelapse/timelapse1K.h264");
}

TEST_F(Figure14cTestFixture, testSelectAngle_2K) {
    testDegenerateAngularSelect("timelapse/timelapse2K.h264");
}

TEST_F(Figure14cTestFixture, testSelectAngle_4K) {
    testDegenerateAngularSelect("timelapse/timelapse4K.h264");
}

/*
TEST_F(DegenerateSelectionBenchmarkTestFixture, testSelect_2K) {
    testSelect("timelapse", 2, 2730, 1024, 1920, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect("timelapse", 2, 2730, 1024, 1920, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect("timelapse", 2, 2730, 1024, 1920, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect("timelapse", 2, 2730, 1024, 1920, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}

TEST_F(DegenerateSelectionBenchmarkTestFixture, testSelect_4K) {
    testSelect("timelapse", 4, 2730, 2048, 3840, {0, temptodouble(pi_div_2)}, {0, temptodouble(pi_div_2)});
    testSelect("timelapse", 4, 2730, 2048, 3840, {0, temptodouble(pi_div_4)}, {0, temptodouble(pi_div_4)});

    testSelect("timelapse", 4, 2730, 2048, 3840, {temptodouble(pi_div_2), temptodouble(pi)}, {temptodouble(pi_div_2), temptodouble(pi)});
    testSelect("timelapse", 4, 2730, 2048, 3840, {temptodouble(pi_div_4), temptodouble(pi)}, {temptodouble(pi_div_4), temptodouble(pi)});
}
*/