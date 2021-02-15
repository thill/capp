#ifndef __CAPP__WORKER_H
#define __CAPP__WORKER_H

#include <functional>
#include <memory>
#include <string>

namespace capp {

class Worker {

private:
  std::string _name;
  std::function<int()> _worker;

public:
  Worker(const std::string_view& name, std::function<int()> worker) : _name(name), _worker(worker){};
  template <typename _WorkerType>
  Worker(const std::string_view& name, std::shared_ptr<_WorkerType> worker)
      : _name(name), _worker([worker]() { return (*worker)(); }){};
  template <typename _WorkerType>
  Worker(const std::string_view& name, std::unique_ptr<_WorkerType> worker)
      : _name(name), _worker([worker]() { return (*worker)(); }){};
  const std::string& name() {
    return _name;
  }
  int operator()() {
    return _worker();
  }
};

} // namespace capp

#endif