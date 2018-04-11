#include "VisitorTemp2.h"
#include <gtest/gtest.h>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::planning;
using namespace lightdb::catalog;

class VisitorTestFixture : public testing::Test {
public:
    VisitorTestFixture()
            : catalog("resources")
    { Catalog::instance(catalog); }

protected:
    Catalog catalog;
};

//#include "VisitorTemp.h"

 TEST_F(VisitorTestFixture, testFoo) {
    auto l = Scan("red10");
    auto r = Scan("red10");
    auto u = l.Union(r);

    Plan plan;

    ChooseDecoders c(plan);
    u->accept(c);
    //ChooseDecoders c;
    //PlanNode<logical::CompositeLightField> n(u);
    //c.dispatch(n);
}
