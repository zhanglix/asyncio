#pragma once

#include <experimental/coroutine>

#include <asyncio/config.hpp>

#include "log.hpp"

BEGIN_ASYNCIO_NAMESPACE;

struct promise_base;
struct done_suspend {
  done_suspend(promise_base *p) : promise(p) {}
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::experimental::coroutine_handle<>) const noexcept;
  void await_resume() const noexcept {}
  promise_base *promise;
};

END_ASYNCIO_NAMESPACE;