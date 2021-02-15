#ifndef __CAPP__IDLE_H
#define __CAPP__IDLE_H

#include <functional>
#include <thread>

namespace capp::idle {

class BusySpin {
public:
  void operator()() {
    asm volatile("pause\n" : : : "memory");
  }
};

class NoOp {
public:
  void operator()() {
  }
};

class Yield {
public:
  void operator()() {
    std::this_thread::yield();
  }
};

class Sleep {
public:
  Sleep() = delete;
  Sleep(const std::chrono::duration<long, std::milli>& duration) : _duration(duration){};
  void operator()() {
    try {
      std::this_thread::sleep_for(_duration);
    } catch (...) {
      // stop blocking
    }
  }

private:
  const std::chrono::duration<long, std::milli> _duration;
};

} // namespace capp::idle

#endif