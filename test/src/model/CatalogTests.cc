#include "Catalog.h"
#include "LightField.h"
#include "TestResources.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::catalog;

class CatalogTestFixture : public testing::Test {
public:
    CatalogTestFixture() = default;
};

TEST_F(CatalogTestFixture, testAmbientCatalog) {
    Catalog::instance(Catalog{Resources.catalog_name});
    ASSERT_NO_THROW(Catalog::instance());
    ASSERT_NO_THROW(Catalog::instance().get(Resources.red10.name));
}

TEST_F(CatalogTestFixture, testCatalogConstructor) {
    Catalog catalog{Resources.catalog_name};
}

TEST_F(CatalogTestFixture, testCatalogRetrieval) {
    Catalog catalog{Resources.catalog_name};
    ASSERT_NO_THROW(catalog.get(Resources.red10.name));

    auto l = catalog.get(Resources.red10.name);

    ASSERT_EQ(l->parents().size(), 0);
}

TEST_F(CatalogTestFixture, testInvalidName) {
    Catalog catalog{Resources.catalog_name};
    ASSERT_THROW(catalog.get("does not exist"), lightdb::errors::_CatalogError);
    ASSERT_THROW(catalog.get("odd characters #%@%^#$"), errors::_CatalogError);
}
