#include "Operators.h"
#include "Physical.h"
#include <gtest/gtest.h>
#include <AssertVideo.h>

using namespace visualcloud;
using namespace std::chrono;

class UnionBenchmarkTestFixture: public testing::Test {
public:
    UnionBenchmarkTestFixture()
        : name("result"),
          pi(102928, 32763),
          pi_div_2(102928, 2*32763),
          pi_div_4(102928, 4*32763)
    { }

    const char *name;
    const rational pi, pi_div_4, pi_div_2;

    //TODO fix
    double temptodouble(const rational &value) {
        return (double)value.numerator() / value.denominator();
    }

    std::string filename(const char *prefix, const size_t size, const size_t duration) {
        return std::string("resources/") + prefix + '-' + std::to_string(size) +
                "K-" + std::to_string(duration) + "s.h264";
    }

    void testUnion(const std::vector<std::string> &filenames) {
        auto red = Decode<EquirectangularGeometry>("resources/red10.h264").apply();
        auto green = Decode<EquirectangularGeometry>("resources/green10.h264").apply();

        auto result = (red | green) >> Encode<YUVColorSpace>();

        ASSERT_GT(result->bytes()->size(), 0);
        ASSERT_EQ(*result->bytes(), *SingletonFileEncodedLightField::create("resources/red10-green10.h264")->bytes());
    }

};

TEST_F(UnionBenchmarkTestFixture, testUnion1) {
    testUnion({});
}
