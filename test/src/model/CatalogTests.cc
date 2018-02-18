#include "Catalog.h"
#include "LightField.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::catalog;

class CatalogTestFixture : public testing::Test {
public:
    CatalogTestFixture() = default;
};

TEST_F(CatalogTestFixture, testNoAmbientCatalog) {
    ASSERT_THROW(Catalog::instance(), errors::_CatalogError);
}

TEST_F(CatalogTestFixture, testAmbientCatalog) {
    Catalog::instance(Catalog{"resources"});
    ASSERT_NO_THROW(Catalog::instance());
    ASSERT_NO_THROW(Catalog::instance().get("red10"));
}

TEST_F(CatalogTestFixture, testCatalogConstructor) {
    Catalog catalog{"resources"};
}

TEST_F(CatalogTestFixture, testCatalogRetrieval) {
    Catalog catalog{"resources"};
    ASSERT_NO_THROW(catalog.get("red10"));

    auto l = catalog.get("red10");

    ASSERT_EQ(l->parents().size(), 0);
}

TEST_F(CatalogTestFixture, testInvalidName) {
    Catalog catalog{"resources"};
    ASSERT_THROW(catalog.get("does not exist"), lightdb::errors::_CatalogError);
    ASSERT_THROW(catalog.get("odd characters #%@%^#$"), errors::_CatalogError);
}
