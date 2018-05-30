#include "HeuristicOptimizer.h"
#include "Coordinator.h"
#include "Display.h"
#include "VisitorTemp2.h"
#include <gtest/gtest.h>
#include <string>

using namespace lightdb;
using namespace lightdb::logical;
using namespace lightdb::optimization;
using namespace lightdb::catalog;
using namespace lightdb::execution;



class TranscodeOptimizerTestFixture : public testing::Test {
 public:

  static std::string VIDEO_NAME;

  
  TranscodeOptimizerTestFixture()
      : catalog("resources")
  { Catalog::instance(catalog);}

  void testGOP(const size_t gop) {
    std::string filename(VIDEO_NAME + std::to_string(gop) + "g");
    auto source = Scan(filename); //std::string("test" + std::to_string(gop)));
    auto encoded = source.Encode("hevc");

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();

    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    print_plan(plan);

    coordinator.save(plan, "out.hevc");
  }

  void testResolution(const size_t resolution) {
    std::string filename(VIDEO_NAME + std::to_string(resolution) + "K.h264");
    auto source = Scan(filename); //std::string("test" + std::to_string(gop)));
    auto encoded = source.Encode("hevc");

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();

    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    print_plan(plan);

    coordinator.save(plan, "out.hevc");
  }

  void testLength(const size_t length) {
    std::string filename(VIDEO_NAME + std::to_string(length) + "seconds.h264");
    auto source = Scan(filename); //std::string("test" + std::to_string(gop)));
    auto encoded = source.Encode("hevc");

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();

    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    print_plan(plan);

    coordinator.save(plan, "out.hevc");
  }

  void testQP(const size_t qp) {
    std::string filename(VIDEO_NAME + std::to_string(qp) + "qp.h264");
    auto source = Scan(filename); //std::string("test" + std::to_string(gop)));
    auto encoded = source.Encode("hevc");

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();

    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    print_plan(plan);

    coordinator.save(plan, "out.hevc");
  }

  void testBF(const size_t bf) {
    std::string filename(VIDEO_NAME + std::to_string(bf) + "bf.h264");
    auto source = Scan(filename); //std::string("test" + std::to_string(gop)));
    auto encoded = source.Encode("hevc");

    auto environment = LocalEnvironment();
    auto coordinator = Coordinator();

    Plan plan = HeuristicOptimizer(environment).optimize(encoded);

    print_plan(plan);

    coordinator.save(plan, "out.hevc");
  }

 protected:
  Catalog catalog;
};

std::string TranscodeOptimizerTestFixture::VIDEO_NAME = "coaster";

// TEST_F(TranscodeOptimizerTestFixture, testGOP60) {
//   testGOP(60);
// }

TEST_F(TranscodeOptimizerTestFixture, testGOP45) {
  testGOP(45);
}

 TEST_F(TranscodeOptimizerTestFixture, testGOP30) {
    testGOP(30);
 }

// TEST_F(TranscodeOptimizerTestFixture, testGOP15) {
//    testGOP(15);
// }


TEST_F(TranscodeOptimizerTestFixture, testTranscode) {
  std::string name;
  std::string testa("coaster45g");
  name = testa;
  auto source = Scan(name);
  auto encoded = source.Encode("hevc");

  auto environment = LocalEnvironment();
  auto coordinator = Coordinator();

  Plan plan = HeuristicOptimizer(environment).optimize(encoded);

  print_plan(plan);

  coordinator.save(plan, "out.hevc");
}
