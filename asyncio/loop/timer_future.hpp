#pragma once

#include <exception>
#include <future>
#include <utility>

#include <asyncio/common.hpp>
#include <asyncio/log.hpp>

#include "future.hpp"
#include "loop_exception.hpp"
#include "timer_handle.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class R, class F> class TimerFuture : public Future<R> {
public:
  TimerFuture(F &f) : _f(f), _handle(nullptr), _refCount(1) {
    _future = std::move(_promise.get_future());
  }
  void setHandle(TimerHandle *handle) { _handle = handle; }
  virtual ~TimerFuture() {
    if (_handle) {
      _handle->subRef();
    }
  }
  virtual R get() override { return _future.get(); }

  void operator()() {
    try {
      setPromise<R>();
    } catch (...) {
      _promise.set_exception(std::current_exception());
    }
    subRef();
  }

  template <class T>
  typename std::enable_if_t<std::is_void<T>::value> setPromise() {
    _f();
    _promise.set_value();
  }

  template <class T>
  typename std::enable_if_t<!std::is_void<T>::value> setPromise() {
    _promise.set_value(_f());
  }

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
    auto timerFuture = (TimerFuture<R, F> *)(handle->data());
    (*timerFuture)();
  }

  size_t refCount() const { return _refCount; }

  size_t addRef() { return ++_refCount; }
  size_t subRef() {
    if (--_refCount == 0) {
      delete this;
      return 0;
    } else {
      return _refCount;
    }
  }

protected:
  std::promise<R> _promise;
  std::future<R> _future;
  F _f;
  TimerHandle *_handle;
  size_t _refCount;
};

template <class R, class F>
class TimerFutureThreadSafe : public TimerFuture<R, F> {
public:
  TimerFutureThreadSafe(F &f) : TimerFuture<R, F>(f) {}
  void release() override {
    this->_handle->loopCore()->callSoonThreadSafe(subRefOnLoop, this);
  }
  static void subRefOnLoop(TimerHandle *handle) {
    auto fut = (TimerFuture<R, F> *)(handle->data());
    fut->TimerFuture<R, F>::release();
    handle->subRef();
  }
};

END_ASYNCIO_NAMESPACE;