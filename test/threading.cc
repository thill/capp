#include "capp/capp.h"
#include "capp/stderr_logger.h"
#include "catch2/catch.hpp"
#include <chrono>

using namespace capp;

constexpr int sleep_millis = 100;

class CountingWorker {
public:
  std::atomic<size_t> count = 0;
  int operator()() {
    count++;
    return 1;
  }
};

TEST_CASE("Multiple Threads Single Worker", "[threading]") {
  auto t1w = std::make_shared<CountingWorker>();
  auto t2w = std::make_shared<CountingWorker>();
  Application app;
  app.set_logger(std::make_shared<StderrLogger>())
      .set_default_idle_strategy([]() {})
      .add_thread({"t1", t1w})
      .add_thread({"t2", t2w})
      .start();
  std::this_thread::sleep_for(std::chrono::milliseconds(sleep_millis));
  REQUIRE(t1w->count > 1);
  REQUIRE(t2w->count > 1);
}

TEST_CASE("Single Thread Single Worker", "[threading]") {
  auto w = std::make_shared<CountingWorker>();
  Application app;
  app.set_logger(std::make_shared<StderrLogger>()).set_default_idle_strategy([]() {});
  app.add_thread({"t1", w}).start();
  std::this_thread::sleep_for(std::chrono::milliseconds(sleep_millis));
  REQUIRE(w->count > 1);
}

TEST_CASE("One Thread Multiple Workers", "[threading]") {
  auto t1w1 = std::make_shared<CountingWorker>();
  auto t1w2 = std::make_shared<CountingWorker>();
  auto t2w1 = std::make_shared<CountingWorker>();
  auto t2w2 = std::make_shared<CountingWorker>();
  {
    Application app;
    app.set_logger(std::make_shared<StderrLogger>())
        .set_default_idle_strategy([]() {})
        .add_thread("t1", {{"w1", t1w1}, {"w2", t1w2}})
        .add_thread("t2", {{"w1", t2w1}, {"w2", t2w2}})
        .start();
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_millis));
    // stops app gracefully before comparing worker counts
  }
  REQUIRE(t1w1->count > 1);
  REQUIRE(t1w1->count == t1w2->count);
  REQUIRE(t2w1->count > 1);
  REQUIRE(t2w1->count == t2w2->count);
}
