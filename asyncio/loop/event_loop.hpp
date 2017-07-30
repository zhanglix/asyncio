#pragma once
#include <chrono>
#include <functional>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include <asyncio/common.hpp>

#include "future.hpp"
#include "loop_core.hpp"
#include "timer_future.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class EventLoop {
public:
  EventLoop(LoopCore *lc = nullptr) : _lc(lc) {}
  virtual void runUntilComplete(FutureBase *future);

  template <class F, class... Args> auto callSoon(F &&f, Args &&... args) {
    return callLater(0, std::forward<F>(f), std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  auto callSoonThreadSafe(F &&f, Args &&... args) {
    typedef typename std::result_of<F(Args...)>::type R;
    auto call = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto timerFuture = new TimerFutureThreadSafe<R, decltype(call)>(call);
    auto handle = _lc->callSoon(timerFuture->callback, timerFuture);
    timerFuture->setHandle(handle);
    return timerFuture;
  }

  template <class F, class... Args>
  auto callLater(uint64_t milliseconds, F &&f, Args &&... args) {
    typedef typename std::result_of<F(Args...)>::type R;
    auto call = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto timerFuture = new TimerFuture<R, decltype(call)>(call);
    auto handle =
        milliseconds > 0
            ? _lc->callLater(milliseconds, timerFuture->callback, timerFuture)
            : _lc->callSoon(timerFuture->callback, timerFuture);
    timerFuture->setHandle(handle);
    return timerFuture;
  }

protected:
  static void call(TimerHandle *handle);

  // virtual void runForever();
  // virtual bool isRunning();
  // virtual void stop();
  // virtual bool isClosed();
  // virtual void closed();
  //  virtual void shutdownAsyncGens();
  // virtual Handle callSoon(std::function<void()> call);
  // virtual Handle callSoonThreadSafe(std::function<void()> call);
  // virtual Handle callLater(uint64_t milliseconds, std::function<void()>
  // call); virtual uint64_t time(); virtual Future *create_future(); virtual
  // Task *createTask(std::function<AWaitable()>);

private:
  LoopCore *_lc;
};

END_ASYNCIO_NAMESPACE;
