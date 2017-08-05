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

  bool done() const override { return _handle->done(); }

  void release() override {
    LOG_DEBUG("fut({})->_refCount:({}). handle({})->refCount:({})",
              (void *)this, _refCount, (void *)_handle, _handle->refCount());
    subRef();
  }

  bool cancel() override {
    if (tryCancelTimer()) {
      endTimer();
      return true;
    } else {
      return false;
    }
  }

  static void callback(TimerHandle *handle) { // timer entry
    auto timerFuture = (TimerFutureBase<R> *)(handle->data());
    timerFuture->process();
  }

  virtual void process() {
    startTimer();
    endTimer();
  }

  virtual void startTimer() = 0;

  virtual bool tryCancelTimer() {
    if (_handle->cancel()) {
      _promise.set_exception(
          std::make_exception_ptr(FutureCanceledError("canceled")));
      return true;
    } else {
      return false;
    }
  }

  virtual void endTimer() {
    tryCallDoneCallback();
    this->subRef();
  }

  using DoneCallback = typename Future<R>::DoneCallback;
  void setDoneCallback(DoneCallback callback) override {
    if (callNowOrSet(callback)) {
      callback(this);
    }
  }

  virtual bool callNowOrSet(DoneCallback &callback) {
    if (done()) {
      return true;
    } else {
      _doneCallback = callback;
      return false;
    }
  }

  void tryCallDoneCallback() {
    if (hasDoneCallback()) {
      _doneCallback(this);
    }
  }

  virtual bool hasDoneCallback() { return (bool)_doneCallback; }

  size_t refCount() const { return _refCount; }

  size_t addRef() { return doAddRef(); }
  size_t subRef() {
    if (doSubRef() == 0) {
      delete this;
      return 0;
    } else {
      return refCount();
    }
  }

  virtual size_t doAddRef() { return ++_refCount; }
  virtual size_t doSubRef() { return --_refCount; }

  void setHandle(TimerHandle *handle) { _handle = handle; }

protected:
  std::promise<R> _promise;
  std::future<R> _future;
  TimerHandle *_handle;
  size_t _refCount;
  DoneCallback _doneCallback;
};

template <class R> class TimerFutureBaseThreadSafe : public TimerFutureBase<R> {
public:
  TimerFutureBaseThreadSafe() : TimerFutureBase<R>() {}
  virtual ~TimerFutureBaseThreadSafe() {}

  virtual size_t doAddRef() override {
    std::lock_guard<std::mutex> lock(_mutex);
    return TimerFutureBase<R>::doAddRef();
  }
  virtual size_t doSubRef() override {
    std::lock_guard<std::mutex> lock(_mutex);
    return TimerFutureBase<R>::doSubRef();
  }

  using DoneCallback = typename Future<R>::DoneCallback;
  virtual bool callNowOrSet(DoneCallback &callback) override {
    std::lock_guard<std::mutex> lock(_mutex);
    return TimerFutureBase<R>::callNowOrSet(callback);
  }

  virtual bool hasDoneCallback() override {
    std::lock_guard<std::mutex> lock(_mutex);
    return TimerFutureBase<R>::hasDoneCallback();
  }

protected:
  std::mutex _mutex;
};

template <class R, bool threadSafe>
class TimerFuture
    : public std::conditional_t<threadSafe, TimerFutureBaseThreadSafe<R>,
                                TimerFutureBase<R>> {
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

  virtual void startTimer() override {
    try {
      this->template setPromise<R>();
    } catch (...) {
      this->_promise.set_exception(std::current_exception());
    }
  }

protected:
  F _f;
};

END_ASYNCIO_NAMESPACE;