#pragma once

#include <experimental/coroutine>
#include <future>
#include <type_traits>

#include <asyncio/common.hpp>
#include <asyncio/log.hpp>

#include "allocator.hpp"
#include "coro.hpp"
#include "promise.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <typename ValueType, typename AllocatorType = DefaultAllocator>
class co_gen {
public:
  using handle_base = std::experimental::coroutine_handle<>;

  class promise_type : public yield_promise<coro<ValueType, AllocatorType>>,
                       public AllocatorType {
  public:
    auto get_return_object() {
      return co_gen(
          std::experimental::coroutine_handle<promise_type>::from_promise(
              *this));
    }
    static auto get_return_object_on_allocation_failure() {
      return co_gen(nullptr);
    }
  };

  using handle_type = std::experimental::coroutine_handle<promise_type>;

  class iterator : public yield_iterator<ValueType, promise_type> {
  public:
    iterator(handle_type handle = nullptr)
        : yield_iterator<ValueType, promise_type>(handle) {}

    coro<void, AllocatorType> operator++() {
      if (this->next()) {
        this->_value = co_await this->_handle.promise().get_yield_value();
      }
    }
  };

  coro<iterator, AllocatorType> begin() const {
    LOG_DEBUG("co_gen begin. coro this: {}, handle: {}", (void *)this,
              _handle.address());
    auto iter = iterator(_handle);
    co_await++ iter;
    co_return iter;
  }

  iterator end() const { return iterator(nullptr); }

  operator bool() const { return bool(_handle); }

  co_gen(handle_type h) : _handle(h) {
    LOG_DEBUG("Constructing co_gen. this: () handle: ()", (void *)this,
              h.address());
  }
  co_gen(const co_gen &) = delete;
  co_gen(co_gen &&other) : _handle(other._handle) {
    LOG_DEBUG("Move Constructing co_gen. this: (), handle: ()", (void *)this,
              _handle.address());
    other._handle = nullptr;
  }
  co_gen() : _handle(nullptr) {}
  void destroy_handle() {
    if (_handle) {
      LOG_DEBUG("Destroying handle: {}", _handle.address());
      _handle.destroy();
    }
    _handle = nullptr;
  }
  ~co_gen() {
    LOG_DEBUG("Destructing co_gen. this: {}, handle: {}", (void *)this,
              _handle.address());
    destroy_handle();
  }

  co_gen &operator=(co_gen &&other) {
    LOG_DEBUG("Move assignment co_gen. this: {}, handle: {}}", (void *)this,
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
  handle_type _handle;
};

END_ASYNCIO_NAMESPACE;