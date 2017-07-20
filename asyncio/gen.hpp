#pragma once

#include <experimental/coroutine>
#include <future>
#include <type_traits>

#include "log.hpp"
#include "promise.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <typename ValueType> class gen {
public:
  using handle_base = std::experimental::coroutine_handle<>;

  class promise_type : public promise<void> {
  public:
    using suspend_never = std::experimental::suspend_never;
    struct yield_suspend : public suspend_always {};
    promise_type() : _current_ready(false) {
      LOG_DEBUG("Constructing promise: 0x{:x}", (long)this);
    }
    ~promise_type() { LOG_DEBUG("Destructing promise: 0x{:x}", (long)this); }

    auto get_return_object() {
      return gen<ValueType>(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }

    auto yield_value(ValueType value) {
      _current_ready = true;
      _current_value = value;
      return yield_suspend{};
    }

    auto get_current_value() const { return _current_value; }

    static auto get_return_object_on_allocation_failure() {
      return gen<ValueType>(nullptr);
    }
    void clear_current() { _current_ready = false; }
    bool has_current() { return _current_ready; }

    template <typename AnyType> inline AnyType await_transform(AnyType any) {
      static_assert(
          std::is_same<yield_suspend, AnyType>::value,
          "operator 'co_await' is not allowed in a gen<ValueType> coroutine!");
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

    void operator++() {
      promise_type &promise = _handle.promise();
      promise.clear_current();
      _handle.resume();
      promise.check_exception();
      if (!promise.has_current()) {
        LOG_DEBUG("has no current value. set this iterator to end state");
        _handle = nullptr;
      }
    }

  private:
    handle_type _handle;
  };

  iterator begin() const {
    auto iter = iterator(_handle);
    ++iter;
    return iter;
  }
  iterator end() const { return iterator(nullptr); }

  operator bool() const { return bool(_handle); }

  gen(std::experimental::coroutine_handle<promise_type> h) : _handle(h) {
    LOG_DEBUG("Constructing gen. this: 0x{:x} handle: 0x{:x}", (long)this,
              (long)h.address());
  }
  gen(const gen &) = delete;
  gen(gen &&other) : _handle(other._handle) {
    LOG_DEBUG("Move Constructing gen. this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)_handle.address());
    other._handle = std::experimental::coroutine_handle<promise_type>();
  }
  gen() : _handle(nullptr) {}
  void destroy_handle() {
    if (_handle) {
      LOG_DEBUG("Destroying handle: 0x{:x}", (long)_handle.address());
      _handle.destroy();
    }
    _handle = nullptr;
  }
  ~gen() {
    LOG_DEBUG("Destructing gen. this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)_handle.address());
    destroy_handle();
  }

  gen &operator=(gen &&other) {
    LOG_DEBUG("Move assignment gen. this: 0x{:x}, handle: 0x{:x}", (long)this,
              (long)other._handle.address());
    destroy_handle();
    _handle = other._handle;
    other._handle = std::experimental::coroutine_handle<promise_type>();
    return *this;
  }

private:
  handle_type _handle;
};

END_ASYNCIO_NAMESPACE;