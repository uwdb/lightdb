#include "Operators.h"
#include "AssertUtility.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::catalog;
//using namespace lightdb::functor;

class AlgebraTestFixture : public testing::Test {
public:
    AlgebraTestFixture()
        : catalog("resources")
    { Catalog::instance(catalog); }

protected:
    Catalog catalog;
};

TEST_F(AlgebraTestFixture, testScanAmbient) {
    auto l = Scan("red10");

    ASSERT_TYPE(*l, ScannedLightField);
    ASSERT_EQ(l->volume().components().size(), 1);
    ASSERT_EQ(l->volume().bounding().theta, AngularRange::ThetaMax);
    ASSERT_EQ(l->volume().bounding().phi, AngularRange::PhiMax);
}

TEST_F(AlgebraTestFixture, testScanExplicit) {
    Catalog catalog("resources");
    auto l = Scan(catalog, "red10");

    ASSERT_TYPE(*l, ScannedLightField);
    ASSERT_EQ(l->volume().components().size(), 1);
    ASSERT_EQ(l->volume().bounding().theta, AngularRange::ThetaMax);
    ASSERT_EQ(l->volume().bounding().phi, AngularRange::PhiMax);
}

TEST_F(AlgebraTestFixture, testScanInvalid) {
    ASSERT_THROW(Scan("does not exist"), lightdb::errors::_CatalogError);
}

TEST_F(AlgebraTestFixture, testSelect) {
    auto l = Scan("red10").Select(Point6D::Zero);

    ASSERT_TYPE(*l, SubsetLightField);
    ASSERT_EQ(l->volume().components().size(), 1);
    ASSERT_EQ(l->volume().bounding(), Point6D::Zero);
}

TEST_F(AlgebraTestFixture, testUnion) {
    auto left = Scan("red10");
    auto right = Scan("red10");
    auto unioned = left.Union(right);

    ASSERT_TYPE(*unioned, CompositeLightField);
    ASSERT_EQ(unioned->volume().components().size(), 2);
    ASSERT_EQ(unioned->parents().size(), 2);
    ASSERT_TYPE(*unioned->parents()[0], ScannedLightField);
    ASSERT_TYPE(*unioned->parents()[1], ScannedLightField);
    ASSERT_EQ(unioned->volume().bounding(), left->volume().bounding() | right->volume().bounding());
    ASSERT_EQ(unioned->volume().components().size(), 2);
    ASSERT_EQ(unioned->volume().components()[0], left->volume().bounding());
    ASSERT_EQ(unioned->volume().components()[1], right->volume().bounding());
}

TEST_F(AlgebraTestFixture, testUnionVector) {
    auto left = Scan("red10");
    auto right = Scan("red10");
    auto unioned = left.Union({right});

    ASSERT_TYPE(*unioned, CompositeLightField);
    ASSERT_EQ(unioned->parents().size(), 2);
    ASSERT_TYPE(*unioned->parents()[0], ScannedLightField);
    ASSERT_TYPE(*unioned->parents()[1], ScannedLightField);
    ASSERT_EQ(unioned->volume().components().size(), 2);
    ASSERT_EQ(unioned->volume().components().size(), 2);
    ASSERT_EQ(unioned->volume().components()[0], left->volume().bounding());
    ASSERT_EQ(unioned->volume().components()[1], right->volume().bounding());
}

TEST_F(AlgebraTestFixture, testRotate) {
    auto l = Scan("red10").Rotate(1, 2);

    ASSERT_TYPE(*l, RotatedLightField);
    ASSERT_EQ(l->downcast<RotatedLightField>().offset().theta, 1);
    ASSERT_EQ(l->downcast<RotatedLightField>().offset().phi, 2);
    ASSERT_EQ(l->volume().components().size(), 1);
}

TEST_F(AlgebraTestFixture, testPartition) {
    // Interval of zero is invalid
    ASSERT_THROW(Scan("red10").Partition(Dimension::Time, 0), lightdb::errors::_InvalidArgument);

    auto l = Scan("red10").Select(SpatiotemporalDimension::Time, {0, 10}).Partition(Dimension::Time, 1);

    ASSERT_TYPE(*l, PartitionedLightField);
    ASSERT_EQ(l->downcast<PartitionedLightField>().dimension(), Dimension::Time);
    ASSERT_EQ(l->downcast<PartitionedLightField>().interval(), 1);
    ASSERT_EQ(l->volume().components().size(), 10);
}

TEST_F(AlgebraTestFixture, testInterpolate) {
    auto l = Scan("red10")
            .Select(SpatiotemporalDimension::Time, {0, 10})
            .Interpolate(Dimension::Time, interpolation::NearestNeighbor());

    ASSERT_TYPE(*l, InterpolatedLightField);
    ASSERT_TYPE(l->downcast<InterpolatedLightField>().interpolator(), interpolation::NearestNeighbor);
    ASSERT_EQ(l->volume().components().size(), 1);
}

TEST_F(AlgebraTestFixture, testDiscretize) {
    auto l = Scan("red10")
            .Select(SpatiotemporalDimension::Time, {0, 10})
            .Discretize(Dimension::Time, 1);

    ASSERT_TYPE(*l, DiscretizedLightField);
    ASSERT_TRUE(l->downcast<DiscretizedLightField>().geometry().is<IntervalGeometry>());
    ASSERT_EQ(l->downcast<DiscretizedLightField>().geometry().downcast<IntervalGeometry>().dimension(), Dimension::Time);
    ASSERT_EQ(l->downcast<DiscretizedLightField>().geometry().downcast<IntervalGeometry>().interval(), 1);
    ASSERT_EQ(l->volume().components().size(), 1);
}

TEST_F(AlgebraTestFixture, testMap) {
    auto l = Scan("red10")
            .Select(SpatiotemporalDimension::Time, {0, 10})
            .Map(Greyscale());

    ASSERT_TYPE(*l, TransformedLightField);
    ASSERT_TRUE(l->downcast<TransformedLightField>().functor().is<Greyscale>());
    ASSERT_EQ(l->volume().components().size(), 1);
}
