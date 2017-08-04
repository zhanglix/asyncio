#pragma once

#include <experimental/coroutine>

#include <asyncio/common.hpp>
#include <asyncio/log.hpp>

#include "../coro/allocator.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class T, class A = DefaultAllocator> class SubRefCoro {
public:
  using suspend_always = std::experimental::suspend_always;
  class SubRefSuspend : public suspend_always {
  public:
    SubRefSuspend(T *p) : _p(p) {}
    void await_suspend(std::experimental::coroutine_handle<>) const noexcept {
      LOG_DEBUG("SubRefSuspend:{} SubRef T*={}", (void *)this, (void *)_p);
      _p->subRef();
    }

  private:
    T *_p;
  };

  class promise_type {
  public:
    promise_type() : _p(nullptr) {}
    ~promise_type() {}
    auto get_return_object() {
      return SubRefCoro(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }
    static auto get_return_object_on_allocation_failure() {
      return SubRefCoro(nullptr);
    }
    suspend_always initial_suspend() { return suspend_always{}; }
    SubRefSuspend final_suspend() { return SubRefSuspend(_p); }
    void setP(T *p) { _p = p; }
    void return_void() {}
    void unhandled_exception() {}

  private:
    T *_p;
  };

  operator bool() const { return bool(_handle); }

  void run() {
    LOG_DEBUG("SubRefCoro::run. this: {}, handle: {}", (void *)this,
              _handle.address());
    if (_handle) {
      _handle.resume();
    } else {
      throw std::invalid_argument("handle is nullptr!");
    }
  }

  void destroy_handle() {
    if (_handle) {
      LOG_DEBUG("Destroying handle: {}", _handle.address());
      _handle.destroy();
    }
    _handle = nullptr;
  }

  SubRefCoro(std::experimental::coroutine_handle<promise_type> h) : _handle(h) {
    LOG_DEBUG("Constructing SubRefCoro this: {} handle: {}", (void *)this,
              h.address());
  }
  SubRefCoro(const SubRefCoro &) = delete;
  SubRefCoro(SubRefCoro &&other) : _handle(other._handle) {
    LOG_DEBUG("Move Constructing SubRefCoro this: {}, handle: {}", (void *)this,
              _handle.address());
    other._handle = std::experimental::coroutine_handle<promise_type>();
  }
  SubRefCoro() {}
  ~SubRefCoro() {
    LOG_DEBUG("Destructing SubRefCoro this: {}, handle: {}", (void *)this,
              _handle.address());
    destroy_handle();
  }

  SubRefCoro &operator=(SubRefCoro &&other) {
    LOG_DEBUG("Move assignment SubRefCoro this: {}, handle: {}", (void *)this,
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

  void setP(T *p) { _handle.promise().setP(p); }

private:
  std::experimental::coroutine_handle<promise_type> _handle;
};

END_ASYNCIO_NAMESPACE;