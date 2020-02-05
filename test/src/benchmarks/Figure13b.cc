#include "HeuristicOptimizer.h"
#include "extension.h"
#include "Greyscale.h"
#include "Ffmpeg.h"
#include "TestResources.h"
#include "AssertVideo.h"
#include "AssertTime.h"
#include "AssertUtility.h"
#include "RequiresGPUTest.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;
using namespace lightdb::video::ffmpeg;
using namespace std::chrono;

class Figure13b: public testing::Test {
public:
    Figure13b()
            : path(std::filesystem::absolute(LIGHTDB_BENCHMARK_DATASET_PATH)) {
        google::InstallFailureSignalHandler();
        LOG(ERROR) << "Before optimizer init\n";//TODO foo
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
        LOG(ERROR) << "After optimizer init\n";//TODO foo
    }

    void testMapGreyscale(const std::string &dataset) {
        auto filename = path / dataset;

        REQUIRE_TIMELAPSE_DATASET();
        //REQUIRE_GPU();

        auto configuration = GetStreamConfiguration(filename, 0, true);
        auto frames = COUNT_FRAMES(filename);

LOG(ERROR)<<"Before query\n";//TODO foo
        auto query = Load(filename, Volume::zero(),  GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()))
                .Map(Greyscale)
                .Encode()
                .Save(Resources.out.h264);

        LOG(ERROR) << "Before execution\n";//TODO foo
        LOG_DURATION(dataset, Coordinator().execute(query));

        LOG(ERROR)<<"After execution\n";;//TODO foo
        EXPECT_VIDEO_VALID(Resources.out.h264);
        EXPECT_VIDEO_FRAMES(Resources.out.h264, frames);
        EXPECT_VIDEO_RESOLUTION(Resources.out.h264, configuration.decode.height, configuration.decode.width);
        EXPECT_EQ(remove(Resources.out.h264), 0);
    }

    void testMapGaussianBlur(const std::string &dataset) {
        auto filename = path / dataset;
        auto configuration = GetStreamConfiguration(filename, 0, true);
        auto frames = COUNT_FRAMES(filename);

        auto blur = lightdb::extensibility::Load("blur");

        auto query = Load(filename, Volume::zero(),  GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()))
                .Map(blur)
                .Encode()
                .Save(Resources.out.h264);

        LOG_DURATION(dataset, Coordinator().execute(query));

        EXPECT_VIDEO_VALID(Resources.out.h264);
        EXPECT_VIDEO_FRAMES(Resources.out.h264, frames);
        EXPECT_VIDEO_RESOLUTION(Resources.out.h264, configuration.decode.height, configuration.decode.width);
        EXPECT_EQ(remove(Resources.out.h264), 0);
    }

protected:
    const std::filesystem::path path;
};

TEST_F(Figure13b, testMap_1K) {
    testMapGreyscale("timelapse/timelapse1K.h264");
}

TEST_F(Figure13b, testMap_2K) {
    testMapGreyscale("timelapse/timelapse2K.h264");
}

TEST_F(Figure13b, testMap_4K) {
    testMapGreyscale("timelapse/timelapse4K.h264");
}

TEST_F(Figure13b, testMapBlur_4K) {
    testMapGaussianBlur("timelapse/timelapse4K.h264");
}
