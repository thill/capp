#ifndef __CAPP__CAPP_WORKER_LIST_H
#define __CAPP__CAPP_WORKER_LIST_H

#include "capp/logger.h"
#include "capp/worker.h"
#include "fmt/format.h"
#include <functional>
#include <stdexcept>
#include <vector>

namespace capp::detail {

class WorkerList {
private:
  std::vector<Worker> _workers;

public:
  WorkerList() = default;

  WorkerList(std::initializer_list<Worker>&& workers) {
    for (auto&& worker : workers) {
      _workers.emplace_back(std::move(worker));
    }
  }

  WorkerList& add(Worker&& worker) {
    _workers.emplace_back(std::forward<Worker>(worker));
    return *this;
  }

  int operator()() {
    int result;
    for (auto& worker : _workers) {
      int worker_result = worker();
      if (worker_result < 0) {
        throw std::runtime_error(fmt::format("worker {} returned exit code {}", worker.name(), worker_result));
      } else if (worker_result > 0) {
        result++;
      }
    }
    return result;
  }
};

} // namespace capp::detail

#endif