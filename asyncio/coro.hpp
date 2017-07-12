#pragma once

#include <experimental/coroutine>

#include "log.hpp"
#include "promise.hpp"
#include <asyncio/config.hpp>

BEGIN_ASYNCIO_NAMESPACE;

template <typename ReturnType> struct coro;

// template <typename ReturnType> struct promise_typed_base : public
// promise_base {
//   static coro<ReturnType> get_return_object_on_allocation_failure();
// };

template <typename ReturnType> struct promise : public promise_base {
  void return_value(ReturnType value) {
    current_value = value;
    this->resume_caller();
  }
  ReturnType get_current_value() {
    this->check_exception();
    return current_value;
  }
  ReturnType current_value;
};

template <> struct promise<void> : public promise_base {
  //  coro<void> get_return_object() ;
  void return_void() { this->resume_caller(); }
  void get_current_value() { this->check_exception(); }
};

template <typename ReturnType> struct coro {
  //  using handle = std::experimental::coroutine_handle<promise_type>;
  using handle_base = std::experimental::coroutine_handle<>;

  struct promise_type : public promise<ReturnType> {
    promise_type() { LOG_DEBUG("Constructing promise: 0x{:x}", (long)this); }
    ~promise_type() { LOG_DEBUG("Desstructing promise: 0x{:x}", (long)this); }

    auto get_return_object() {
      return coro<ReturnType>(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }
    static auto get_return_object_on_allocation_failure() {
      return coro<ReturnType>(nullptr);
    }
  };

  bool await_ready() const noexcept { return false; }
  bool await_suspend(handle_base caller_handle) {
    _handle.promise().set_caller_handle(caller_handle);
    _handle.resume();
    return _handle.promise().need_suspend();
  }
  ReturnType await_resume() const {
    return _handle.promise().get_current_value();
  }

  void destroy_handle() {
    if (_handle) {
      LOG_DEBUG("Destroying handle: 0x{:x}", (long)_handle.address());
      _handle.destroy();
    }
    _handle = nullptr;
  }

  coro(std::experimental::coroutine_handle<promise_type> h) : _handle(h) {}
  coro(const coro &) = delete;
  coro(coro &&other) : _handle(other._handle) { other._handle = nullptr; }
  ~coro() { destroy_handle(); }

  coro &operator=(coro &&other) {
    _handle = other._handle;
    other._handle = nullptr;
    return *this;
  }

private:
  std::experimental::coroutine_handle<promise_type> _handle;
};
END_ASYNCIO_NAMESPACE;