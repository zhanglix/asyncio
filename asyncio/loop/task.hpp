#pragma once

#include <asyncio/common.hpp>

#include <asyncio/coroutine.hpp>

#include "end_coro.hpp"
#include "future.hpp"
#include "timer_future.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class C, class R, bool threadSafe, class A = DefaultAllocator>
class Task : public std::conditional_t<threadSafe, TimerFutureBaseThreadSafe<R>,
                                       TimerFutureBase<R>> {
public:
  Task(C &co) : _co(std::move(co)), _done(false) {}
  Task(C &&co) : Task(co) {}

  virtual ~Task() {}

  bool done() const override { return this->_done; }
  void process() override { startTimer(); }
  void startTimer() override {
    _coHolder = runCoro();
    _coHolder.run();
  }
  void endTimer() override {
    this->_done = true;
    TimerFutureBase<R>::endTimer();
  }

  EndCoro<A> runCoro() {
    try {
      co_await setPromise<R>();
    } catch (...) {
      this->_promise.set_exception(std::current_exception());
    }
    co_return([&] { this->endTimer(); });
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
  EndCoro<A> _coHolder;
  bool _done;
};

END_ASYNCIO_NAMESPACE;