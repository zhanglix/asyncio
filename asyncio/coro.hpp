#pragma once
#include <asyncio/config.hpp>

#include <experimental/coroutine>
#include <future>

#include "log.hpp"
#include "promise.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <typename ReturnType> class coro {
public:
  using handle_base = std::experimental::coroutine_handle<>;

  class promise_type : public promise<ReturnType> {
  public:
    promise_type() { LOG_DEBUG("Constructing promise: 0x{:x}", (long)this); }
    ~promise_type() { LOG_DEBUG("Destructing promise: 0x{:x}", (long)this); }

    auto get_return_object() {
      return coro<ReturnType>(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }
    static auto get_return_object_on_allocation_failure() {
      return coro<ReturnType>(nullptr);
    }
  };

  operator bool() const { return bool(_handle); }

  bool await_ready() const noexcept {
    LOG_DEBUG("await_ready. coro this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)_handle.address());
    return _handle.promise().ready();
  }

  bool await_suspend(handle_base caller_handle) {
    LOG_DEBUG("await_suspend. coro this: 0x{:x}, "
              "handle: 0x{:x} caller_handle: 0x{:x}",
              (long)this, (long)_handle.address(),
              (long)caller_handle.address());
    _handle.resume();
    if (_handle.promise().need_suspend()) {
      _handle.promise().set_caller_handle(caller_handle);
      return true;
    } else {
      return false;
    }
  }

  ReturnType await_resume() const {
    LOG_DEBUG("await_resume. coro this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)_handle.address());
    return _handle.promise().get_return_value();
  }

  void destroy_handle() {
    if (_handle) {
      LOG_DEBUG("Destroying handle: 0x{:x}", (long)_handle.address());
      _handle.destroy();
    }
    _handle = nullptr;
  }

  coro(std::experimental::coroutine_handle<promise_type> h) : _handle(h) {
    LOG_DEBUG("Constructing coro. this: 0x{:x} handle: 0x{:x}", (long)this,
              (long)h.address());
  }
  coro(const coro &) = delete;
  coro(coro &&other) : _handle(other._handle) {
    LOG_DEBUG("Move Constructing coro. this: 0x{:x}, handle: 0x{:x}",
              (long)this, (long)_handle.address());
    other._handle = std::experimental::coroutine_handle<promise_type>();
  }
  coro() {}
  ~coro() {
    LOG_DEBUG("Destructing coro. this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)_handle.address());
    destroy_handle();
  }

  coro &operator=(coro &&other) {
    LOG_DEBUG("Move assignment coro. this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)other._handle.address());
    destroy_handle();
    _handle = other._handle;
    other._handle = std::experimental::coroutine_handle<promise_type>();
    return *this;
  }

private:
  std::experimental::coroutine_handle<promise_type> _handle;
};

template <typename ReturnType> class co_runner {
public:
  co_runner(coro<ReturnType> &co) : _runner(run(co)) {
    _runner.await_suspend(nullptr);
  }
  co_runner(coro<ReturnType> &&co) : _runner(run(co)) {
    _runner.await_suspend(nullptr);
  }
  co_runner(co_runner &) = delete;
  co_runner(co_runner &&) = delete;
  std::future<ReturnType> get_future() { return std::move(_future); }

private:
  coro<void> _runner;
  std::future<ReturnType> _future;

  coro<void> run(coro<ReturnType> &co) {
    std::promise<ReturnType> promise;
    this->_future = promise.get_future();
    try {
      ReturnType ret = co_await std::move(co);
      promise.set_value(ret);
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
  }
};

template <> coro<void> inline co_runner<void>::run(coro<void> &co) {
  std::promise<void> promise;
  this->_future = promise.get_future();
  try {
    coro<void> co_holder = std::move(co);
    co_await co_holder;
    promise.set_value();
  } catch (...) {
    promise.set_exception(std::current_exception());
  }
}

END_ASYNCIO_NAMESPACE;