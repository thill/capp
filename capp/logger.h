#ifndef __CAPP__STDERR_LOGGER_H
#define __CAPP__STDERR_LOGGER_H

#include <iostream>

namespace capp {

class Logger {
public:
  virtual ~Logger() = default;
  virtual void info(const std::string_view& message) = 0;
  virtual void error(const std::string_view& message) = 0;
};

class NoopLogger : public Logger {
public:
  virtual ~NoopLogger() = default;
  virtual void info(const std::string_view&) override {
  }
  virtual void error(const std::string_view&) override {
  }
};

} // namespace capp

#endif