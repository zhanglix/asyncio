#pragma once

#include <experimental/coroutine>
#include <future>
#include <type_traits>

#include "allocator.hpp"
#include "log.hpp"
#include "promise.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <typename ValueType, typename AllocatorType = DefaultAllocator>
class gen {
public:
  using handle_base = std::experimental::coroutine_handle<>;

  class promise_type : public yield_promise<ValueType>, public AllocatorType {
  public:
    auto get_return_object() {
      return gen(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }

    static auto get_return_object_on_allocation_failure() {
      return gen(nullptr);
    }
  };

  using handle_type = std::experimental::coroutine_handle<promise_type>;

  class iterator : public yield_iterator<ValueType, promise_type> {
  public:
    iterator(handle_type handle = nullptr)
        : yield_iterator<ValueType, promise_type>(handle) {}

    void operator++() {
      if (this->next()) {
        this->_value = this->_handle.promise().get_yield_value();
      }
    }
  };

  iterator begin() const {
    auto iter = iterator(_handle);
    ++iter;
    return iter;
  }
  iterator end() const { return iterator(nullptr); }

  operator bool() const { return bool(_handle); }

  gen(handle_type h) : _handle(h) {
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

  void *handle_address() {
    if (_handle) {
      return _handle.address();
    } else {
      return nullptr;
    }
  }

private:
  handle_type _handle;
};

END_ASYNCIO_NAMESPACE;