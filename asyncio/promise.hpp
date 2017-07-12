#pragma once

#include "suspendable.hpp"
#include <asyncio/config.hpp>
#include <experimental/coroutine>

#include "log.hpp"

BEGIN_ASYNCIO_NAMESPACE;

struct promise_base {
  using coroutine_handle = std::experimental::coroutine_handle<>;
  using suspend_always = std::experimental::suspend_always;

  promise_base() : done(false) {}

  suspend_always initial_suspend() { return suspend_always{}; }
  done_suspend final_suspend();
  void unhandled_exception() { exception_caught = std::current_exception(); }

  void set_caller_handle(coroutine_handle &h) { caller_handle = h; }

  bool need_suspend() {
    check_exception();
    return !done;
  }
  void check_exception() {
    if (exception_caught) {
      std::rethrow_exception(exception_caught);
    }
  }

  void set_done() {
    LOG_DEBUG("promise done: {0:x}", long(this));
    done = true;
  }

  ~promise_base() { LOG_DEBUG("Destructing promise_base: {0:x}", long(this)); }

protected:
  coroutine_handle caller_handle;
  void resume_caller() {
    if (caller_handle) {
      LOG_DEBUG("resume caller: 0x{:x}", long(caller_handle.address()));
      caller_handle.resume();
    }
  }
  std::exception_ptr exception_caught;
  bool done;
};

END_ASYNCIO_NAMESPACE;