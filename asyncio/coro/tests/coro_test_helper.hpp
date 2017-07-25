#pragma once
#include <experimental/coroutine>
#include <string>

#include <asyncio/log.hpp>

struct leak_base {
  leak_base(std::string name = "leak") : name(name) {}
  bool await_ready() const noexcept {
    LOG_DEBUG("leak({}) will never ready");
    return false;
  }
  void await_suspend(std::experimental::coroutine_handle<> h) noexcept {
    this->handle = h;
    this->address = (long)h.address();
    LOG_DEBUG("leak({}) will suspend handle: 0x{:x} ...", this->name,
              this->address);
  }
  void resume_caller() {
    LOG_DEBUG("leak({}) resuming handle: 0x{:x} ...", this->name,
              this->address);
    if (this->handle) {
      this->handle.resume();
    }
    LOG_DEBUG("leak({}) resuming handle: 0x{:x} DONE!", this->name,
              this->address);
  }
  std::experimental::coroutine_handle<> handle;
  long address;
  std::string name;
};

template <typename T> struct handle_leak : public leak_base {
  handle_leak(T v = T(), std::string name = "leak")
      : leak_base(name), value(v) {}
  T await_resume() const noexcept { return this->value; }

  T value;
};

template <> struct handle_leak<void> : public leak_base {
  handle_leak(std::string name = "leak_void") : leak_base(name) {}
  void await_resume() const noexcept {}
};
