#pragma once

#include <asyncio/config.hpp>
#include <experimental/coroutine>

#include "log.hpp"

BEGIN_ASYNCIO_NAMESPACE;

struct done_suspend;
struct promise_base {
  using coroutine_handle = std::experimental::coroutine_handle<>;
  using suspend_always = std::experimental::suspend_always;

  promise_base() : done(false) {}

  suspend_always initial_suspend() {
    LOG_DEBUG("create initial_suspend(). promise this: 0x{:x}", (long)this);
    return suspend_always{};
  }
  done_suspend final_suspend();
  void unhandled_exception() { exception_caught = std::current_exception(); }

  void set_caller_handle(coroutine_handle &h) {
    LOG_DEBUG("set_caller_handle. promise this: 0x{:x} handle: 0x{:x}",
              (long)this, (long)h.address());
    caller_handle = h;
  }

  bool need_suspend() {
    check_exception();
    return !done;
  }
  void check_exception() {
    if (exception_caught) {
      std::rethrow_exception(exception_caught);
    }
  }

  void resume_caller() {
    if (caller_handle) {
      LOG_DEBUG("resume caller. promise this: 0x{:x}, caller_handle: 0x{:x}",
                (long)this, (long)caller_handle.address());
      caller_handle.resume();
    }
  }

  void set_done() {
    LOG_DEBUG("promise done. this: 0x{:x}", long(this));
    done = true;
  }

  ~promise_base() { LOG_DEBUG("Destructing promise_base: {0:x}", long(this)); }

protected:
  coroutine_handle caller_handle;
  std::exception_ptr exception_caught;
  bool done;
};

struct done_suspend {
  done_suspend(promise_base *p) : promise(p) {
    LOG_DEBUG("constructing done_suspend. this: 0x{:x}, promise: 0x{:x}",
              (long)this, (long)p);
  }
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::experimental::coroutine_handle<>) const noexcept;
  void await_resume() const noexcept {}
  promise_base *promise;
};

// imlementations following ...
done_suspend promise_base::final_suspend() {
  LOG_DEBUG("create final_suspend(). promise this: 0x{:x}", (long)this);
  return done_suspend(this);
}

void done_suspend::await_suspend(std::experimental::coroutine_handle<>) const
    noexcept {
  LOG_DEBUG("Done! will resume_caller()."
            "this: 0x{:x}, promise: 0x{:x}",
            (long)this, (long)promise);
  promise->resume_caller();
  //  LOG_DEBUG("caller resumed. this: 0x{:x}", (long)this);
}

END_ASYNCIO_NAMESPACE;