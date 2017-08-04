#pragma once

#include <exception>
#include <future>
#include <mutex>
#include <utility>

#include <asyncio/common.hpp>
#include <asyncio/log.hpp>

#include "future.hpp"
#include "loop_exception.hpp"
#include "timer_handle.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class R> class TimerFutureBase : public Future<R> {
public:
  TimerFutureBase() : _handle(nullptr), _refCount(1) {
    _future = std::move(_promise.get_future());
  }

  virtual ~TimerFutureBase() {
    if (_handle) {
      _handle->subRef();
    }
  }
  virtual R get() override { return _future.get(); }

  virtual void operator()() = 0;

  bool completed() override { return _handle->completed(); }
  bool cancel() override {
    if (_handle->cancel()) {
      _promise.set_exception(
          std::make_exception_ptr(FutureCanceledError("canceled")));
      subRef();
      return true;
    } else {
      return false;
    }
  }

  void release() override {
    LOG_DEBUG("fut({})->_refCount:({}). handle({})->refCount:({})",
              (void *)this, _refCount, (void *)_handle, _handle->refCount());
    subRef();
  }

  static void callback(TimerHandle *handle) {
    auto timerFuture = (TimerFutureBase<R> *)(handle->data());
    (*timerFuture)();
  }

  size_t refCount() const { return _refCount; }

  virtual size_t addRef() { return ++_refCount; }
  virtual size_t subRef() {
    if (--_refCount == 0) {
      delete this;
      return 0;
    } else {
      return _refCount;
    }
  }
  void setHandle(TimerHandle *handle) { _handle = handle; }

protected:
  std::promise<R> _promise;
  std::future<R> _future;
  TimerHandle *_handle;
  size_t _refCount;
};

template <class R> class TimerFuture : public TimerFutureBase<R> {
public:
  typedef std::function<R(void)> F;
  TimerFuture(F &f) : _f(f) {}
  virtual ~TimerFuture() {}

  template <class T>
  typename std::enable_if_t<std::is_void<T>::value> setPromise() {
    _f();
    this->_promise.set_value();
  }

  template <class T>
  typename std::enable_if_t<!std::is_void<T>::value> setPromise() {
    this->_promise.set_value(_f());
  }

  virtual void operator()() {
    try {
      this->template setPromise<R>();
    } catch (...) {
      this->_promise.set_exception(std::current_exception());
    }
    this->subRef();
  }

protected:
  F _f;
};

template <class R> class TimerFutureThreadSafe : public TimerFuture<R> {
public:
  using typename TimerFuture<R>::F;
  TimerFutureThreadSafe(F &f) : TimerFuture<R>(f) {}

  virtual size_t addRef() override {
    std::lock_guard<std::mutex> lock(_mutex);
    return TimerFuture<R>::addRef();
  }
  virtual size_t subRef() override {
    std::lock_guard<std::mutex> lock(_mutex);
    return TimerFuture<R>::subRef();
  }

protected:
  std::mutex _mutex;
};

END_ASYNCIO_NAMESPACE;