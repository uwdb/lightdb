#include "Geometry.h"
#include <gtest/gtest.h>

using namespace lightdb;

class GeometryTestFixture : public testing::Test {
public:
    GeometryTestFixture() = default;
};

TEST_F(GeometryTestFixture, testCompositeVolumeIterator) {
    auto v = Volume::limits();

    // Spatial
    v.x({0, 10});
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::X, 1)).size(),  10);
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::X, 10)).size(),  1);

    // Angular
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::Theta, 1)).size(),  7);
    v.theta({0, 3});
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::Theta, 1)).size(),  3);
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::Theta, 3)).size(),  1);

    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::Phi, 1)).size(),  4);
    v.phi({0, 3});
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::Phi, 1)).size(),  3);
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::Phi, 3)).size(),  1);

    // Temporal
    v.t({0, 10});
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::Time, 1)).size(),  10);
    ASSERT_EQ(static_cast<std::vector<Volume>>(v.partition(Dimension::Time, 10)).size(),  1);
}
