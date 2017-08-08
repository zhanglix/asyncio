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
  typedef std::conditional_t<threadSafe, TimerFutureBaseThreadSafe<R>,
                             TimerFutureBase<R>>
      BaseClass;
  Task(LoopCore *lc, uint64_t later, C &co)
      : BaseClass(lc, later), _co(std::move(co)) {}
  Task(LoopCore *lc, uint64_t later, C &&co) : Task(lc, later, co) {}

  void reset(uint64_t later, C &co) {
    BaseClass::reset(later);
    _co = std::move(co);
  }

  bool executeTimer() final {
    _coHolder = runCoro();
    _coHolder.run();
    return false;
  }

  void cleanUp() final {
    _coHolder = EndCoro<A>();
    _co = C();
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
};

END_ASYNCIO_NAMESPACE;