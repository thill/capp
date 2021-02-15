#include "capp/capp.h"
#include "capp/stderr_logger.h"
#include "catch2/catch.hpp"
#include <chrono>

using namespace capp;

constexpr int sleep_millis = 100;

class ExitCodeWorker {
private:
  int _exit_code;

public:
  ExitCodeWorker(int exit_code) : _exit_code(exit_code){};
  int operator()() {
    return _exit_code;
  }
};

class ThrowingWorker {
private:
  std::runtime_error _ex;

public:
  ThrowingWorker(std::runtime_error ex) : _ex(ex){};
  int operator()() {
    throw _ex;
  }
};

TEST_CASE("Handle Worker Exit Code", "[exceptions]") {
  std::function<void()> idle = []() {};
  auto w = std::make_shared<ExitCodeWorker>(-234);
  Application app;
  app.set_logger(std::make_shared<StderrLogger>())
      .set_default_idle_strategy([]() {})
      .add_thread({"t1", w})
      .start()
      .join();
  REQUIRE(app.exit_code() == -234);
}

TEST_CASE("Handle Worker Exception", "[exceptions]") {
  std::function<void()> idle = []() {};
  auto w = std::make_shared<ThrowingWorker>(std::runtime_error("blow up"));
  Application app;
  app.set_logger(std::make_shared<StderrLogger>())
      .set_default_idle_strategy([]() {})
      .add_thread({"t1", w})
      .start()
      .join();
  REQUIRE(app.exit_code() == std::numeric_limits<int>::min());
}

TEST_CASE("Worker List Exit Code Exception", "[exceptions]") {
  std::function<void()> idle = []() {};
  auto w1 = std::make_shared<ExitCodeWorker>(-234);
  auto w2 = std::make_shared<ExitCodeWorker>(-234);
  Application app;
  app.set_logger(std::make_shared<StderrLogger>())
      .set_default_idle_strategy([]() {})
      .add_thread("t1", {{"w1", w1}, {"w2", w2}})
      .start()
      .join();
  REQUIRE(app.exit_code() == std::numeric_limits<int>::min());
}
