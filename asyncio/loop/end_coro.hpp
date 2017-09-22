#pragma once

#include <experimental/coroutine>

#include <asyncio/common.hpp>
#include <asyncio/log.hpp>

#include "../coro/allocator.hpp"
#include <functional>

BEGIN_ASYNCIO_NAMESPACE;

template <class A = DefaultAllocator> class EndCoro {
public:
  using suspend_always = std::experimental::suspend_always;
  class EndSuspend : public suspend_always {
  public:
    EndSuspend(std::function<void()> &f) : _f(f) {}
    void await_suspend(std::experimental::coroutine_handle<>) const noexcept {
      ASYNCIO_DEBUG("EndSuspend::await_suspend ");
      _f();
    }

  private:
    std::function<void()> &_f;
  };

  class promise_type {
  public:
    auto get_return_object() {
      return EndCoro(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }
    static auto get_return_object_on_allocation_failure() {
      return EndCoro(nullptr);
    }
    suspend_always initial_suspend() { return suspend_always{}; }
    EndSuspend final_suspend() { return EndSuspend(_f); }
    void return_value(std::function<void()> &&f) { return_value(f); }
    void return_value(std::function<void()> &f) { _f = f; }
    void unhandled_exception() {
      ASYNCIO_ERROR("Should catch all exceptions in EndCoro!");
    }

  private:
    std::function<void()> _f;
  };

  operator bool() const { return bool(_handle); }

  void run() {
    ASYNCIO_DEBUG("EndCoro::run. this: {}, handle: {}", (void *)this,
              _handle.address());
    if (_handle) {
      _handle.resume();
    } else {
      throw std::invalid_argument("handle is nullptr!");
    }
  }

  void destroy_handle() {
    if (_handle) {
      ASYNCIO_DEBUG("Destroying handle: {}", _handle.address());
      _handle.destroy();
    }
    _handle = nullptr;
  }

  EndCoro(std::experimental::coroutine_handle<promise_type> h) : _handle(h) {
    ASYNCIO_DEBUG("Constructing EndCoro this: {} handle: {}", (void *)this,
              h.address());
  }
  EndCoro(const EndCoro &) = delete;
  EndCoro(EndCoro &&other) : _handle(other._handle) {
    ASYNCIO_DEBUG("Move Constructing EndCoro this: {}, handle: {}", (void *)this,
              _handle.address());
    other._handle = std::experimental::coroutine_handle<promise_type>();
  }
  EndCoro() {}
  ~EndCoro() {
    ASYNCIO_DEBUG("Destructing EndCoro this: {}, handle: {}", (void *)this,
              _handle.address());
    destroy_handle();
  }

  EndCoro &operator=(EndCoro &&other) {
    ASYNCIO_DEBUG("Move assignment EndCoro this: {}, handle: {}", (void *)this,
              other._handle.address());
    destroy_handle();
    _handle = other._handle;
    other._handle = std::experimental::coroutine_handle<promise_type>();
    return *this;
  }

  void *handle_address() {
    if (_handle) {
      return _handle.address();
    } else {
      return nullptr;
    }
  }

private:
  std::experimental::coroutine_handle<promise_type> _handle;
};

END_ASYNCIO_NAMESPACE;