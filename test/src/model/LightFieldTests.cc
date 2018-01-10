#include "Operators.h"
#include <fstream>
#include <gtest/gtest.h>

class LightFieldTestFixture : public testing::Test {
public:
    LightFieldTestFixture()
    { }
};

TEST_F(LightFieldTestFixture, testPanoramicConstructor) {
    PlanarTiledVideoLightField("resources/test-lightfield.h265",
                               Volume{{0, 1}, {0, 1}, {0, 0}, TemporalRange::TemporalMax,
                                      AngularRange::ThetaMax, AngularRange::PhiMax},
                               3, 3);
}

TEST_F(LightFieldTestFixture, testValueOutOfVolume) {
    auto field = PlanarTiledVideoLightField("resources/test-lightfield.h265",
                                            Volume{{0, 1}, {0, 1}, {0, 0}, TemporalRange::TemporalMax,
                                                   AngularRange::ThetaMax, AngularRange::PhiMax},
                                            3, 3);

    ASSERT_EQ(field.value({-1,  0,  0,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0, -1,  0,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0, -1,  0,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0, -1,  0,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0,  0, -1,  0}), YUVColor::Null);
    ASSERT_EQ(field.value({ 0,  0,  0,  0,  0, -1}), YUVColor::Null);
    ASSERT_EQ(field.value({ 1,  1,  1,  1,  1,  1}), YUVColor::Null);
    ASSERT_EQ(field.value({-1, -1, -1, -1, -1, -1}), YUVColor::Null);
}
