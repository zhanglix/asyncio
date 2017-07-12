#pragma once
#include <experimental/coroutine>

template <typename T> struct handle_leak {
  handle_leak() {}
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::experimental::coroutine_handle<> h) noexcept {
    this->handle = h;
  }
  int await_resume() const noexcept { return this->value; }
  std::experimental::coroutine_handle<> handle;
  T value;
};