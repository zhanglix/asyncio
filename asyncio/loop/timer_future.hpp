#pragma once

#include <asyncio/common.hpp>
#include <exception>
#include <future>
#include <utility>

#include "future.hpp"
#include "loop_exception.hpp"
#include "timer_handle.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class R, class F> class TimerFuture : public Future<R> {
public:
  TimerFuture(F &f) : _f(f), _handle(nullptr) {
    _future = std::move(_promise.get_future());
  }
  void setHandle(TimerHandle *handle) { _handle = handle; }
  virtual ~TimerFuture() {}
  virtual R get() override { return _future.get(); }

  void operator()() {
    try {
      setPromise<R>();
    } catch (...) {
      _promise.set_exception(std::current_exception());
    }
    assert(_handle);
    release();
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
      release();
      return true;
    } else {
      return false;
    }
  }
  void release() override {
    if (_handle->subRef() == 0) {
      delete this;
    }
  }
  static void callback(TimerHandle *handle) {
    auto timerFuture = (TimerFuture<R, F> *)(handle->data());
    (*timerFuture)();
  }

private:
  std::promise<R> _promise;
  std::future<R> _future;
  F _f;
  TimerHandle *_handle;
};
END_ASYNCIO_NAMESPACE;