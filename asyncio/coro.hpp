#pragma once
#include <asyncio/common.hpp>

#include <experimental/coroutine>
#include <future>

#include "allocator.hpp"
#include "log.hpp"
#include "promise.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <typename ReturnType, typename AllocatorType = DefaultAllocator>
class coro {
public:
  using handle_base = std::experimental::coroutine_handle<>;

  class promise_type : public promise<ReturnType>, public AllocatorType {
  public:
    promise_type() { LOG_DEBUG("Constructing promise: {}", (void *)this); }
    ~promise_type() { LOG_DEBUG("Destructing promise: {}", (void *)this); }

    auto get_return_object() {
      LOG_DEBUG("get_return_object: promise: {}", (void *)this);
      return coro(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }
    static auto get_return_object_on_allocation_failure() {
      return coro(nullptr);
    }
  };

  operator bool() const { return bool(_handle); }

  bool await_ready() const noexcept {
    LOG_DEBUG("await_ready. coro this: {}, handle: {}", (void *)this,
              _handle.address());
    return _handle.promise().ready();
  }

  bool await_suspend(handle_base caller_handle) {
    LOG_DEBUG("await_suspend. coro this: {}, "
              "handle: {} caller_handle: {}",
              (void *)this, _handle.address(), caller_handle.address());
    _handle.resume();
    if (_handle.promise().need_suspend()) {
      _handle.promise().set_caller_handle(caller_handle);
      return true;
    } else {
      return false;
    }
  }

  ReturnType await_resume() const {
    LOG_DEBUG("await_resume. coro this: {}, handle: {}", (void *)this,
              _handle.address());
    return _handle.promise().get_return_value();
  }

  void destroy_handle() {
    if (_handle) {
      LOG_DEBUG("Destroying handle: {}", _handle.address());
      _handle.destroy();
    }
    _handle = nullptr;
  }

  coro(std::experimental::coroutine_handle<promise_type> h) : _handle(h) {
    LOG_DEBUG("Constructing coro. this: {} handle: {}", (void *)this,
              h.address());
  }
  coro(const coro &) = delete;
  coro(coro &&other) : _handle(other._handle) {
    LOG_DEBUG("Move Constructing coro. this: {}, handle: {}", (void *)this,
              _handle.address());
    other._handle = std::experimental::coroutine_handle<promise_type>();
  }
  coro() {}
  ~coro() {
    LOG_DEBUG("Destructing coro. this: {}, handle: {}", (void *)this,
              _handle.address());
    destroy_handle();
  }

  coro &operator=(coro &&other) {
    LOG_DEBUG("Move assignment coro. this: {}, handle: {}", (void *)this,
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