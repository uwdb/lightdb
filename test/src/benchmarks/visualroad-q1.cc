#include "HeuristicOptimizer.h"
#include "Display.h"
#include "AssertUtility.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;

class Q1TestFixture : public testing::Test {
public:
    Q1TestFixture()
            : uadetrac("ua-detrac"),
              random("random"),
              visualroad("visualroad"),
              vrdetrac("vr-detrac"),
              output("output", true) {
        Optimizer::instance<HeuristicOptimizer>(LocalEnvironment());
        Catalog::instance(output);
    }

protected:
    Catalog uadetrac;
    Catalog random;
    Catalog visualroad;
    Catalog vrdetrac;
    Catalog output;
};

TEST_F(Q1TestFixture, testQ1duplicate) {
    REQUIRE_UA_DETRAC_DATASET();

    auto duplicates = 60u;
    auto name = "MVI_40244";
    auto path = uadetrac.path() / name / "stream0.mp4";
    std::vector<LightFieldReference> sinks;

    auto input = Load(path, Volume::zero(), GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()));

    for(auto i = 0u; i < duplicates; i++)
        sinks.emplace_back(
                input.Select(PhiRange{0, rational_times_real({2, 4}, PI)})
                     .Select(SpatiotemporalDimension::Time, {1, 2})
                     .Store(std::string(name) + '-' + std::to_string(i)));

    Coordinator().execute(sinks);
}

TEST_F(Q1TestFixture, testQ1random) {
    REQUIRE_RANDOM_DATASET();

    auto duplicates = 60u;
    auto name = "random960x540x60";
    auto path = random.path() / name / "stream0.h264";
    std::vector<LightFieldReference> sinks;

    auto input = Load(path, Volume::zero(), GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()));

    for(auto i = 0u; i < duplicates; i++)
        sinks.emplace_back(
                input.Select(PhiRange{0, rational_times_real({2, 4}, PI)})
                     .Select(SpatiotemporalDimension::Time, {1, 2})
                     .Store(std::string(name) + '-' + std::to_string(i)));

    Coordinator().execute(sinks);
}

TEST_F(Q1TestFixture, testQ1unique) {
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

    for(const auto &name: names) {
        auto input = Load(vrdetrac.path() / name / "stream0.h264", Volume::zero(), GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()));
        sinks.emplace_back(
                input.Select(PhiRange{0, rational_times_real({2, 4}, PI)})
                     .Store(name + "_out"));
    }

    Coordinator().execute(sinks);
}

TEST_F(Q1TestFixture, testQ1_scale1) {
    REQUIRE_VISUALROAD_DATASET();

    auto duplicates = 4u;
    auto name = "scale1";
    std::vector<LightFieldReference> sinks;

    auto input = Load(visualroad.path() / name / "stream0.h264", Volume::zero(), GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()));

    for(auto i = 0u; i < duplicates; i++)
        sinks.emplace_back(
                input.Select(PhiRange{0, rational_times_real({2, 4}, PI)})
                     .Select(SpatiotemporalDimension::Time, {1, 2})
                     .Store(std::string(name) + '-' + std::to_string(i)));

    Coordinator().execute(sinks);
}

TEST_F(Q1TestFixture, testQ1_scale2) {
    REQUIRE_VISUALROAD_DATASET();

    auto duplicates = 8u;
    auto name = "scale1";
    std::vector<LightFieldReference> sinks;

    auto input = Load(visualroad.path() / name / "stream0.h264", Volume::zero(), GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()));

    for(auto i = 0u; i < duplicates; i++)
        sinks.emplace_back(
                input.Select(PhiRange{0, rational_times_real({2, 4}, PI)})
                        .Select(SpatiotemporalDimension::Time, {1, 2})
                        .Store(std::string(name) + '-' + std::to_string(i)));

    Coordinator().execute(sinks);
}

TEST_F(Q1TestFixture, testQ1_scale4) {
    REQUIRE_VISUALROAD_DATASET();

    auto duplicates = 16u;
    auto name = "scale1";
    std::vector<LightFieldReference> sinks;

    auto input = Load(visualroad.path() / name / "stream0.h264", Volume::zero(), GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()));

    for(auto i = 0u; i < duplicates; i++)
        sinks.emplace_back(
                input.Select(PhiRange{0, rational_times_real({2, 4}, PI)})
                        .Select(SpatiotemporalDimension::Time, {1, 2})
                        .Store(std::string(name) + '-' + std::to_string(i)));

    Coordinator().execute(sinks);
}

TEST_F(Q1TestFixture, testQ1_scale8) {
    REQUIRE_VISUALROAD_DATASET();

    auto duplicates = 32u;
    auto name = "scale1";
    std::vector<LightFieldReference> sinks;

    auto input = Load(visualroad.path() / name / "stream0.h264", Volume::zero(), GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()));

    for(auto i = 0u; i < duplicates; i++)
        sinks.emplace_back(
                input.Select(PhiRange{0, rational_times_real({2, 4}, PI)})
                        .Select(SpatiotemporalDimension::Time, {1, 2})
                        .Store(std::string(name) + '-' + std::to_string(i)));

    Coordinator().execute(sinks);
}

TEST_F(Q1TestFixture, testQ1_scale4_sink) {
    REQUIRE_VISUALROAD_DATASET();

    Catalog::instance(visualroad);

    auto duplicates = 16u;
    auto name = "scale1";
    std::vector<LightFieldReference> sinks;

    auto input = Load(visualroad.path() / name / "stream0.h264", Volume::zero(), GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples()));

    for(auto i = 0u; i < duplicates; i++)
        sinks.emplace_back(
                input.Select(PhiRange{0, rational_times_real({2, 4}, PI)})
                        .Select(SpatiotemporalDimension::Time, {1, 2})
                        .Sink());

    Coordinator().execute(sinks);
}
