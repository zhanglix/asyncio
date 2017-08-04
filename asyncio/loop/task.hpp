#pragma once

#include <asyncio/common.hpp>

#include <asyncio/coroutine.hpp>

#include "future.hpp"
#include "timer_future.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class C, class R> class Task : public TimerFutureBase<R> {
public:
  Task(C &co) : _co(std::move(co)), _done(false) {}
  Task(C &&co) : Task<C, R>(co) {}

  virtual ~Task() {}

  bool completed() override {
    return this->_done && TimerFutureBase<R>::completed();
  }
  void operator()() override {
    _coHolder = runCoro();
    _coHolder.await_suspend(nullptr);
  }

  coro<void> runCoro() {
    try {
      co_await setPromise<R>();
    } catch (...) {
      this->_promise.set_exception(std::current_exception());
    }
    this->_done = true;
    this->subRef();
  }

  template <class T>
  coro<typename std::enable_if_t<std::is_void<T>::value>> setPromise() {
    co_await _co;
    this->_promise.set_value();
  }

  template <class T>
  coro<typename std::enable_if_t<!std::is_void<T>::value>> setPromise() {
    this->_promise.set_value(co_await _co);
  }

protected:
  C _co;
  coro<void> _coHolder;
  bool _done;
};

END_ASYNCIO_NAMESPACE;
