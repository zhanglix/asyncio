#pragma once
#include <exception>

#include <asyncio/common.hpp>

BEGIN_ASYNCIO_NAMESPACE;

class LoopException : public std::runtime_error {
public:
  explicit LoopException(const std::string &what_arg)
      : std::runtime_error(what_arg) {}
  explicit LoopException(const char *what_arg) : std::runtime_error(what_arg) {}
};

class FutureCanceledError : public LoopException {
public:
  explicit FutureCanceledError(const std::string &what_arg)
      : LoopException(what_arg) {}
  explicit FutureCanceledError(const char *what_arg)
      : LoopException(what_arg) {}
};

class LoopBusyError : public LoopException {
public:
  explicit LoopBusyError(const std::string &what_arg)
      : LoopException(what_arg) {}
  explicit LoopBusyError(const char *what_arg) : LoopException(what_arg) {}
};

END_ASYNCIO_NAMESPACE;