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
  TimerFutureBase(LoopCore *lc, uint64_t later)
      : _handle(nullptr), _lc(lc), _later(later) {
    _future = std::move(_promise.get_future());
  }

  void reset(uint64_t later) {
    Future<R>::reset();
    _handle = nullptr;
    _later = later;
    _promise = std::promise<R>();
    _future = std::move(_promise.get_future());
  }

  virtual R get() final { return _future.get(); }

  void release() final { this->subRef(); }

  void setupTimer() { this->startTimer(); } // promote protected

  void doStartTimer() final {
    this->addRef(); // add a ref for LoopCore object
    _handle = startTimerViaLoopCore();
  }

  static void staticEntry(TimerHandle *handle) {
    auto timerFuture = (TimerFutureBase<R> *)(handle->data());
    timerFuture->process();
    timerFuture->subRef(); // release ref if called by loop_core;
  }

  auto getEntry() { return staticEntry; }

  virtual TimerHandle *startTimerViaLoopCore() {
    return _later > 0 ? _lc->callLater(_later, getEntry(), this)
                      : _lc->callSoon(getEntry(), this);
  }

  virtual bool cancelTimer() final {
    if (_handle->cancel()) {
      this->subRef(); // release ref if canceled call from loop_core
    }
    _promise.set_exception(
        std::make_exception_ptr(FutureCanceledError("canceled")));
    return true;
  }

  void afterDone() final {
    auto cb = getDoneCallback();
    if (cb) {
      cb(this);
    }
  }
  using typename FutureBase::DoneCallback;

  virtual DoneCallback getDoneCallback() { return std::move(_doneCallback); }

  void setDoneCallback(DoneCallback callback) final {
    if (callNowOrSet(callback)) {
      callback(this);
    }
  }

  virtual bool callNowOrSet(DoneCallback &callback) {
    if (this->done()) {
      return true;
    } else {
      _doneCallback = std::move(callback);
      return false;
    }
  }

  void recycle() final {
    if (_handle) {
      _handle->subRef();
      _handle = nullptr;
    }
    cleanUp();
    doRecycle();
  }

  virtual void cleanUp() {}
  virtual void doRecycle() { delete this; }

protected:
  std::promise<R> _promise;
  std::future<R> _future;
  DoneCallback _doneCallback;

  LoopCore *_lc;
  TimerHandle *_handle;
  size_t _later;
};

template <class R,
          typename BaseClass = BasicHandleThreadSafe<TimerFutureBase<R>>>
class TimerFutureBaseThreadSafe : public BaseClass {
public:
  using BaseClass::BaseClass;

  using typename FutureBase::DoneCallback;
  virtual bool callNowOrSet(DoneCallback &callback) final {
    std::lock_guard<std::mutex> lock(this->_mutex);
    return BaseClass::callNowOrSet(callback);
  }

  virtual DoneCallback getDoneCallback() final {
    std::lock_guard<std::mutex> lock(this->_mutex);
    return BaseClass::getDoneCallback();
  }

  TimerHandle *startTimerViaLoopCore() final {
    return this->_lc->callSoonThreadSafe(this->getEntry(), this);
  }
};

template <typename R, bool threadSafe>
using TimerFutureBaseType =
    std::conditional_t<threadSafe, TimerFutureBaseThreadSafe<R>,
                       TimerFutureBase<R>>;

template <class R, bool threadSafe>
class TimerFuture : public TimerFutureBaseType<R, threadSafe> {
public:
  typedef std::function<R(void)> F;

  TimerFuture(LoopCore *lc, uint64_t later, F &f)
      : TimerFutureBaseType<R, threadSafe>(lc, later), _f(f) {}

  void reset(uint64_t later, F &f) {
    TimerFutureBaseType<R, threadSafe>::reset(later);
    _f = f;
  }

  bool executeTimer() final {
    try {
      this->template setPromise<R>();
    } catch (...) {
      this->_promise.set_exception(std::current_exception());
    }
    return true;
  }

  template <class T>
  typename std::enable_if_t<std::is_void<T>::value> setPromise() {
    _f();
    this->_promise.set_value();
  }

  template <class T>
  typename std::enable_if_t<!std::is_void<T>::value> setPromise() {
    this->_promise.set_value(_f());
  }

protected:
  F _f;
};

END_ASYNCIO_NAMESPACE;
