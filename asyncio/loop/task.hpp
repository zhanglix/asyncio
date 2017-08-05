#pragma once

#include <asyncio/common.hpp>

#include <asyncio/coroutine.hpp>

#include "future.hpp"
#include "sub_ref_coro.hpp"
#include "timer_future.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class C, class R, class A = DefaultAllocator>
class Task : public TimerFutureBase<R> {
public:
  Task(C &co) : _co(std::move(co)), _done(false) {}
  Task(C &&co) : Task<C, R>(co) {}

  virtual ~Task() {}

  bool done() const override { return this->_done; }
  void processTimer() override { startTimer(); }
  void startTimer() override {
    _coHolder = runCoro();
    _coHolder.setP(this);
    _coHolder.run();
  }
  void endTimer() override {
    this->_done = true;
    TimerFutureBase<R>::endTimer();
  }

  SubRefCoro<Task, A> runCoro() {
    try {
      co_await setPromise<R>();
    } catch (...) {
      this->_promise.set_exception(std::current_exception());
    }
    this->_done = true;
    this->callDoneCallback();
  }

  template <class T>
  coro<typename std::enable_if_t<std::is_void<T>::value>, A> setPromise() {
    co_await _co;
    this->_promise.set_value();
  }

  template <class T>
  coro<typename std::enable_if_t<!std::is_void<T>::value>, A> setPromise() {
    this->_promise.set_value(co_await _co);
  }

protected:
  C _co;
  SubRefCoro<Task, A> _coHolder;
  bool _done;
};

END_ASYNCIO_NAMESPACE;
