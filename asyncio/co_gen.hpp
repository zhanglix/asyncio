#pragma once

#include <experimental/coroutine>
#include <future>
#include <type_traits>

#include "coro.hpp"
#include "log.hpp"
#include "promise.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <typename ValueType> class co_gen {
public:
  using handle_base = std::experimental::coroutine_handle<>;

  class promise_type : public promise<void> {
  public:
    using suspend_never = std::experimental::suspend_never;
    promise_type() : _current_ready(false) {
      LOG_DEBUG("Constructing co_gen::promise: 0x{:x}", (long)this);
    }
    ~promise_type() { LOG_DEBUG("Destructing promise: 0x{:x}", (long)this); }

    auto get_return_object() {
      LOG_DEBUG("get_return_object: promise: 0x{:x}", (long)this);
      return co_gen<ValueType>(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }

    auto yield_value(ValueType value) {
      _current_ready = true;
      _current_value = value;
      return done_suspend(this);
    }

    auto get_current_value() const { return _current_value; }

    static auto get_return_object_on_allocation_failure() {
      return co_gen<ValueType>(nullptr);
    }
    void clear_current() { _current_ready = false; }
    bool has_current() { return _current_ready; }
    bool ready() { return _current_ready || this->done; }

    bool await_ready() {
      LOG_DEBUG("co_gen::promise_type await_ready: promise this: 0x{:x}",
                (long)this);
      return this->ready();
    }
    bool await_suspend(handle_base caller_handle) {
      LOG_DEBUG("co_gen::promise_type await_suspend: promise this: 0x{:x} "
                "caller_handle: 0x{:x}",
                (long)this, (long)caller_handle.address());

      auto &&handle =
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this);
      LOG_DEBUG("co_gen::promise_type after from_promise. promise this: 0x{:x} "
                "caller_handle: 0x{:x}, handle: {}",
                (long)this, (long)caller_handle.address(), handle.address());
      handle.resume();
      LOG_DEBUG("co_gen::promise_type after resume(): promise this: 0x{:x} "
                "caller_handle: 0x{:x}",
                (long)this, (long)caller_handle.address());
      this->check_exception();
      if (this->ready()) {
        return false;
      } else {
        this->set_caller_handle(caller_handle);
        return true;
      }
    }
    void await_resume() {
      LOG_DEBUG("co_gen::promise_type await_resume: promise this: 0x{:x}",
                (long)this);
      this->check_exception();
    }

  private:
    ValueType _current_value;
    bool _current_ready;
  };

  using handle_type = std::experimental::coroutine_handle<promise_type>;

  class iterator {
  public:
    iterator(handle_type handle = nullptr) : _handle(handle) {}
    bool operator!=(const iterator &other) const {
      return _handle != other._handle;
    }

    auto operator*() const { return _handle.promise().get_current_value(); }

    coro<void> operator++() {
      LOG_DEBUG("iterator ++: iter this: 0x{:x}, handle: 0x{:x}", (long)this,
                (long)_handle.address());
      promise_type &promise = _handle.promise();
      promise.clear_current();
      // _handle.resume();
      // promise.check_exception();
      LOG_DEBUG("before co_await promise");
      co_await promise;
      LOG_DEBUG("iterator ++: after co_await promise. "
                "this: 0x{:x}, handle: 0x{:x}",
                (long)this, (long)_handle.address());
      if (!promise.has_current()) {
        LOG_DEBUG("has no current value. set this iterator to end state");
        _handle = nullptr;
      }
    }

  private:
    handle_type _handle;
  };

  coro<iterator> begin() const {
    LOG_DEBUG("co_gen begin. coro this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)_handle.address());
    auto iter = iterator(_handle);
    co_await++ iter;
    LOG_DEBUG("co_gen after ++ iter. coro this: 0x{:x}, handle: 0x{:x}",
              (long)this, (long)_handle.address());
    co_return iter;
  }
  iterator end() const { return iterator(nullptr); }

  operator bool() const { return bool(_handle); }

  co_gen(std::experimental::coroutine_handle<promise_type> h) : _handle(h) {
    LOG_DEBUG("Constructing co_gen. this: 0x{:x} handle: 0x{:x}", (long)this,
              (long)h.address());
  }
  co_gen(const co_gen &) = delete;
  co_gen(co_gen &&other) : _handle(other._handle) {
    LOG_DEBUG("Move Constructing co_gen. this: 0x{:x}, handle: 0x{:x}",
              (long)this, (long)_handle.address());
    other._handle = std::experimental::coroutine_handle<promise_type>();
  }
  co_gen() : _handle(nullptr) {}
  void destroy_handle() {
    if (_handle) {
      LOG_DEBUG("Destroying handle: 0x{:x}", (long)_handle.address());
      _handle.destroy();
    }
    _handle = nullptr;
  }
  ~co_gen() {
    LOG_DEBUG("Destructing co_gen. this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)_handle.address());
    destroy_handle();
  }

  co_gen &operator=(co_gen &&other) {
    LOG_DEBUG("Move assignment co_gen. this: 0x{:x}, handle: 0x{:x}",
              (long)this, (long)other._handle.address());
    destroy_handle();
    _handle = other._handle;
    other._handle = std::experimental::coroutine_handle<promise_type>();
    return *this;
  }

private:
  handle_type _handle;
};

END_ASYNCIO_NAMESPACE;