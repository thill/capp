#ifndef __CAPP__CAPP_DETAIL_H
#define __CAPP__CAPP_DETAIL_H

#include "capp/logger.h"
#include "capp/worker.h"
#include "fmt/format.h"
#include <atomic>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace capp::detail {

class WorkerThread {
private:
  Worker _worker;
  std::atomic<bool>& _run_flag_ref;
  std::function<void()> _idle_strategy;
  std::shared_ptr<Logger> _logger;
  std::atomic<bool> _running = false;
  std::atomic<int> _exit_code = 0;
  std::unique_ptr<std::thread> _thread;

public:
  WorkerThread(Worker&& worker,
               std::atomic<bool>& run_flag_ref,
               std::function<void()> idle_strategy,
               std::shared_ptr<Logger> logger)
      : _worker(std::forward<Worker>(worker)), _run_flag_ref(run_flag_ref), _idle_strategy(idle_strategy),
        _logger(logger){};
  WorkerThread(const WorkerThread&) = delete;

  void start() {
    _logger->info(fmt::format("thread {} - starting", _worker.name()));
    _running.store(true);
    _thread = std::make_unique<std::thread>([this]() { this->run(); });
  }

  bool running() {
    return _running.load();
  }

  void join() {
    join(_idle_strategy);
  }

  void join(std::function<void()>& idle_strategy) {
    if (_thread != nullptr && _thread->joinable()) {
      _thread->join();
    } else {
      while (running())
        idle_strategy();
    }
  }

  int exit_code() {
    return _exit_code;
  }

private:
  void run() {
    _logger->info(fmt::format("thread {} - running", _worker.name()));
    _running.store(true);
    worker_loop();
    _logger->info(fmt::format("thread {} - exited with code {}", _worker.name(), _exit_code));
    _running.store(false);
  }

  void worker_loop() {
    while (_run_flag_ref.load(std::memory_order_relaxed)) {
      do_work();
    }
  }

  void do_work() noexcept {
    int result;
    try {
      result = _worker();
    } catch (std::exception& e) {
      _logger->error(fmt::format("thread {} - threw exception: '{}'", _worker.name(), e.what()));
      result = std::numeric_limits<int>::min();
    } catch (...) {
      _logger->error(fmt::format("thread {} - threw exception", _worker.name()));
      result = std::numeric_limits<int>::min();
    }
    if (result < 0) {
      _logger->error(fmt::format("thread {} - exit code: {}", _worker.name(), result));
      _exit_code = result;
      _run_flag_ref = false;
    } else if (result == 0) {
      _idle_strategy();
    }
  }
};

} // namespace capp::detail

#endif