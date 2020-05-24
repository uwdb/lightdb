#include "HeuristicOptimizer.h"
#include "Greyscale.h"
#include "extension.h"
#include "Display.h"
#include "AssertUtility.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class Q2cTestFixture : public testing::Test {
public:
    Q2cTestFixture()
            : initialized(initialize()),
              catalog("random"),
              visualroad("visualroad") {
              //catalog("ua-detrac") {
        Catalog::instance(catalog);
    }

protected:
    bool initialized;
    Catalog catalog;
    Catalog visualroad;

    static bool initialize() {
        //google::InitGoogleLogging("LightDB");
        google::SetCommandLineOption("GLOG_minloglevel", "5");
        return true;
    }
};

TEST_F(Q2cTestFixture, testQ2cduplicate) {
    REQUIRE_RANDOM_DATASET();

    auto duplicates = 60u;
    //auto name = "MVI_40244";
    auto name = "random960x540x60";
    auto yolo = lightdb::extensibility::Load("yolo");
    std::vector<LightFieldReference> sinks;
    std::vector<std::string> outputs;

    auto mapped = Scan(name).Map(yolo);

    for(auto i = 0u; i < duplicates; i++) {
        sinks.emplace_back(mapped.Encode(Codec::boxes()));
        outputs.emplace_back(std::string("out-") + std::to_string(i));
    }

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(sinks);

    coordinator.save(plan, outputs);
    //GTEST_SKIP(); //TODO
}

TEST_F(Q2cTestFixture, testQ2cunique) {
    REQUIRE_UA_DETRAC_DATASET();

    std::vector<std::string> names = {
            "MVI_20011", "MVI_40152", "MVI_39811", "MVI_40244", "MVI_39781", "MVI_40962", "MVI_40213", "MVI_40141",
            "MVI_40191", "MVI_63562", "MVI_39931", "MVI_39761", "MVI_40871", "MVI_40192", "MVI_40981", "MVI_41073",
            "MVI_40161", "MVI_40171", "MVI_20032", "MVI_40243", "MVI_39771", "MVI_40732", "MVI_40752", "MVI_40172",
            "MVI_20035", "MVI_20062", "MVI_20011", "MVI_63563", "MVI_40204", "MVI_63525", "MVI_63553", "MVI_20051",
            "MVI_40992", "MVI_39801", "MVI_40212", "MVI_63561", "MVI_20065", "MVI_40751", "MVI_20063", "MVI_40991",
            "MVI_20061", "MVI_20012", "MVI_63554", "MVI_40131", "MVI_63521", "MVI_40162", "MVI_20064", "MVI_40211",
            "MVI_40181", "MVI_20033", "MVI_63552", "MVI_39861", "MVI_41063", "MVI_39851", "MVI_63544", "MVI_40963",
            "MVI_39821", "MVI_40201", "MVI_20052", "MVI_20034"};
    std::vector<LightFieldReference> sinks;

    auto yolo = lightdb::extensibility::Load("yolo");

    for(const auto &name: names) {
        auto input = Scan(name);
        sinks.emplace_back(
                input.Map(yolo)
                     .Encode(Codec::boxes()));
    }

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(sinks);

    coordinator.save(plan, {names.size(), "out"});
}

TEST_F(Q2cTestFixture, testQ2c_scale1) {
    REQUIRE_VISUALROAD_DATASET();

    Catalog::instance(visualroad);

    auto duplicates = 4u;
    auto name = "scale1";
    auto yolo = lightdb::extensibility::Load("yolo");
    std::vector<LightFieldReference> sinks;
    std::vector<std::string> outputs;

    auto input = Scan(name);

    for(auto i = 0u; i < duplicates; i++) {
        sinks.emplace_back(input.Map(yolo).Encode(Codec::boxes()));
        outputs.emplace_back(std::string("out-") + std::to_string(i));
    }

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(sinks);

    coordinator.save(plan, outputs);
}

TEST_F(Q2cTestFixture, testQ2c_scale2) {
    REQUIRE_VISUALROAD_DATASET();

    Catalog::instance(visualroad);

    auto duplicates = 8u;
    auto name = "scale1";
    auto yolo = lightdb::extensibility::Load("yolo");
    std::vector<LightFieldReference> sinks;
    std::vector<std::string> outputs;

    auto input = Scan(name);

    for(auto i = 0u; i < duplicates; i++) {
        sinks.emplace_back(input.Map(yolo).Encode(Codec::boxes()));
        outputs.emplace_back(std::string("out-") + std::to_string(i));
    }

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(sinks);

    coordinator.save(plan, outputs);
}

TEST_F(Q2cTestFixture, testQ2c_scale4) {
    REQUIRE_VISUALROAD_DATASET();

    Catalog::instance(visualroad);

    auto duplicates = 16u;
    auto name = "scale1";
    auto yolo = lightdb::extensibility::Load("yolo");
    std::vector<LightFieldReference> sinks;
    std::vector<std::string> outputs;

    auto input = Scan(name);

    for(auto i = 0u; i < duplicates; i++) {
        sinks.emplace_back(input.Map(yolo).Encode(Codec::boxes()));
        outputs.emplace_back(std::string("out-") + std::to_string(i));
    }

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(sinks);

    coordinator.save(plan, outputs);
}

TEST_F(Q2cTestFixture, testQ2c_scale8) {
    REQUIRE_VISUALROAD_DATASET();

    Catalog::instance(visualroad);

    auto duplicates = 32u;
    auto name = "scale1";
    auto yolo = lightdb::extensibility::Load("yolo");
    std::vector<LightFieldReference> sinks;
    std::vector<std::string> outputs;

    auto input = Scan(name);

    for(auto i = 0u; i < duplicates; i++) {
        sinks.emplace_back(input.Map(yolo).Encode(Codec::boxes()));
        outputs.emplace_back(std::string("out-") + std::to_string(i));
    }

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();
    Plan plan = HeuristicOptimizer(environment).optimize(sinks);

    coordinator.save(plan, outputs);
}
