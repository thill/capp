#ifndef __CAPP__CAPP_H
#define __CAPP__CAPP_H

#include "capp/detail/worker_list.h"
#include "capp/detail/worker_thread.h"
#include "capp/idle.h"
#include "capp/logger.h"
#include "capp/worker.h"
#include "fmt/format.h"
#include <atomic>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace capp {

class Application {
private:
  std::vector<std::function<void()>> _start_functions;
  std::vector<std::function<void()>> _join_functions;
  std::vector<std::function<int()>> _exit_code_functions;
  std::atomic<bool> _run_flag = true;
  std::function<void()> _default_idle_strategy = idle::Sleep{std::chrono::milliseconds{1}};
  std::shared_ptr<Logger> _logger = std::make_shared<NoopLogger>();

public:
  Application() = default;
  ~Application() {
    stop();
  }
  Application(Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) = default;
  Application& operator=(Application&&) = default;

  /**
   * Set the default idle strategy to be used when an idle strategy is not explicitly provided.
   */
  Application& set_default_idle_strategy(std::function<void()> default_idle_strategy) {
    _default_idle_strategy = default_idle_strategy;
    return *this;
  }

  /**
   * Set the logging implementation to use for errors and startup/shutdown events. Logging is disabled by default.
   */
  Application& set_logger(const std::shared_ptr<Logger> logger) {
    _logger = logger;
    return *this;
  }

  /** 
   * Get a reference to the internal atomic run flag. This is meant to make integration easier when this framework is 
   * not in control of every single runtime thread.
   */
  std::atomic<bool>& run_flag() {
    return _run_flag;
  }

  /**
   * Add a thread consisting of the given worker and idle strategy
   */
  Application& add_thread(Worker&& worker, std::function<void()> idle_strategy) {
    auto thread =
        std::make_shared<capp::detail::WorkerThread>(std::forward<Worker>(worker), _run_flag, idle_strategy, _logger);
    _start_functions.emplace_back([thread]() { thread->start(); });
    _join_functions.emplace_back([thread]() { thread->join(); });
    _exit_code_functions.emplace_back([thread]() { return thread->exit_code(); });
    return *this;
  }

  /** 
   * Add a thread consisting of the given worker and default idle strategy
   */
  Application& add_thread(Worker&& worker) {
    return add_thread(std::forward<Worker>(worker), _default_idle_strategy);
  }

  /**
   * Add a thread consisting of the given worker list and idle strategy
   */
  Application& add_thread(const std::string_view& thread_name,
                          std::initializer_list<Worker>&& workers,
                          std::function<void()> idle_strategy) {
    return add_thread({thread_name, capp::detail::WorkerList{std::forward<std::initializer_list<Worker>>(workers)}},
                      idle_strategy);
  }

  /**
   * Add a thread consisting of the given worker list and default idle strategy
   */
  Application& add_thread(const std::string_view& thread_name, std::initializer_list<Worker>&& workers) {
    return add_thread({thread_name, capp::detail::WorkerList{std::forward<std::initializer_list<Worker>>(workers)}},
                      _default_idle_strategy);
  }

  /**
   * Start the internal threads. Threads will be stopped when `stop()` is called of when the Application destructor is invoked.
   */
  inline Application& start() {
    for (auto& func : _start_functions)
      func();
    return *this;
  }

  /**
   * Set the run flag to false to asynchronously stop all threads
   */
  inline Application& stop_async() {
    _run_flag = false;
    return *this;
  }

  /**
   * Stop all threads and wait for the threads to gracefully exit before returning
   */
  inline Application& stop() {
    stop_async();
    join();
    return *this;
  }

  /**
   * Get the exit code of the application. Any workers that threw and exception or returned a negative result code will 
   * cause this method to return a non-zero exit code.
   */
  inline int exit_code() {
    for (auto& func : _exit_code_functions) {
      int result = func();
      if (result != 0)
        return result;
    }
    return 0;
  }

  /**
   * Join the application and wait for all threads to exit
   */
  inline int join() {
    for (auto& func : _join_functions)
      func();
    return exit_code();
  }
};

} // namespace capp

#endif