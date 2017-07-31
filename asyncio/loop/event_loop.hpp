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
    return callOnTimer<false, F, Args...>(0, std::forward<F>(f),
                                          std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  auto callSoonThreadSafe(F &&f, Args &&... args) {
    return callOnTimer<true, F, Args...>(0, std::forward<F>(f),
                                         std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  auto callLater(uint64_t milliseconds, F &&f, Args &&... args) {
    return callOnTimer<false, F, Args...>(milliseconds, std::forward<F>(f),
                                          std::forward<Args>(args)...);
  }

  template <bool threadSafe, class F, class... Args>
  auto callOnTimer(uint64_t milliseconds, F &&f, Args &&... args) {
    using R = typename std::result_of<F(Args...)>::type;
    using FutureType = std::conditional_t<threadSafe, TimerFutureThreadSafe<R>,
                                          TimerFuture<R>>;
    std::function<R(void)> call =
        std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    auto timerFuture = new FutureType(call);
    setupFuture<threadSafe, FutureType>(timerFuture, milliseconds);
    return timerFuture;
  }

protected:
  template <bool threadSafe, class Fut>
  std::enable_if_t<threadSafe> setupFuture(Fut *fut, uint64_t) {
    auto handle = _lc->callSoonThreadSafe(fut->callback, fut);
    setupHandle(fut, handle);
  }

  template <bool threadSafe, class Fut>
  std::enable_if_t<!threadSafe> setupFuture(Fut *fut, uint64_t ms) {
    auto handle = ms > 0 ? _lc->callLater(ms, fut->callback, fut)
                         : _lc->callSoon(fut->callback, fut);
    setupHandle(fut, handle);
  }

  template <class Fut> void setupHandle(Fut *fut, TimerHandle *handle) {
    fut->setHandle(handle);
    fut->addRef();
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