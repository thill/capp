#ifndef __CAPP__STDERR_STDERR_LOGGER_H
#define __CAPP__STDERR_STDERR_LOGGER_H

#include "capp/logger.h"
#include "fmt/format.h"
#include <iostream>

namespace capp {

class StderrLogger : public Logger {
public:
  virtual ~StderrLogger() = default;
  virtual void info(const std::string_view& message) override {
    std::cerr << fmt::format("INFO: {}", message) << std::endl;
  }
  virtual void error(const std::string_view& message) override {
    std::cerr << fmt::format("ERROR: {}", message) << std::endl;
  }
};

} // namespace capp

#endif