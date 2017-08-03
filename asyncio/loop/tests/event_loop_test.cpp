#include <catch.hpp>
#include <fakeit.hpp>

#define ENABLE_ASYNCIO_LOG
#include <asyncio/log.hpp>

#include "../event_loop.hpp"
#include "../loop_core.hpp"
#include "../loop_exception.hpp"
#include "../uv_loop_core.hpp"
#include "trivial_loop.hpp"
#include <asyncio/coroutine.hpp>

using namespace std;
using namespace fakeit;
using namespace asyncio;

namespace eventloop_test {

TEST_CASE("event_loop run", "[examples]") {
  EventLoop loop;
  auto fut = loop.callSoon([](int a, int b) { return a + b; }, 3, 5);
  SECTION("run until complete") { loop.runUntilComplete(fut); }
  SECTION("run until stop called") {
    loop.callSoon([&] { loop.stop(); })->release();
    loop.runForever();
  }
  CHECK(fut->get() == 8);
  fut->release();
}

TEST_CASE("event_loop createTask", "[examples]") {
  EventLoop loop;
  auto foo = [](int a, int b) -> coro<int> { co_return a + b; };
  Task<int> *task;
  SECTION("coro<int>&&") { task = loop.createTask(foo(3, 5)); }
  SECTION("coro<int>&") {
    auto coro = foo(3, 5);
    task = loop.createTask(coro);
  }
  loop.runUntilComplete(task);
  CHECK(task->get() == 8);
  task->release();
}

TEST_CASE("event_loop createTask delayed", "[examples]") {
  EventLoop loop;
  AWaitable<void> awaitable;
  auto foo = [&](int a, int b) -> coro<int> {
    co_await awaitable;
    co_return a + b;
  };
  Task<int> *task = loop.createTask(foo(6, 2));
  auto second = loop.callSoon([&] {});
  auto third = loop.callLater(10, [&] { awaitable.resume(); });
  loop.runUntilComplete(second);
  CHECK_FALSE(task->completed());
  loop.runUntilComplete(task);
  CHECK(third->completed());
  CHECK(task->get() == 8);
  task->release();
  second->release();
  third->release();
}

TEST_CASE("eventloop timer", "[loop][trivial]") {
  TrivialLoop trivialLoop;
  Mock<TrivialLoop> spy(trivialLoop);
  Spy(Method(spy, callSoon));
  Spy(Method(spy, recycleTimerHandle));

  LoopCore *lc = &spy.get();
  EventLoop loop(lc, false);

  SECTION("int") {
    auto fut = loop.callSoon([](int in) { return in; }, 3);
    LOG_DEBUG("after callSoon()");
    CHECK_FALSE(fut->completed());
    Verify(Method(spy, callSoon)).Once();
    SECTION("done") {
      LOG_DEBUG("before runOneIteration()");
      loop.runUntilComplete(fut);
      LOG_DEBUG("after runOneIteration()");

      CHECK(fut->completed());
      CHECK(fut->get() == 3);
      CHECK_FALSE(fut->cancel());
    }
    SECTION("canceled") {
      CHECK(fut->cancel());
      CHECK(fut->completed());
      CHECK_THROWS_AS(fut->get(), FutureCanceledError);
    }

    LOG_DEBUG("before fut->release()");
    REQUIRE_NOTHROW(fut->release());
    LOG_DEBUG("after fut->release()");
    Verify(Method(spy, recycleTimerHandle)).Once();
  }

  SECTION("void") {
    int out = 0;
    auto fut = loop.callSoon([](int in, int &out) { out = in; }, 3, ref(out));
    CHECK_FALSE(fut->completed());
    Verify(Method(spy, callSoon)).Once();

    SECTION("done") {
      loop.runUntilComplete(fut);
      CHECK(fut->completed());
      CHECK_NOTHROW(fut->get());
      CHECK_FALSE(fut->cancel());
      CHECK(out == 3);
    }
    SECTION("canceled") {
      CHECK(fut->cancel());
      CHECK(fut->completed());
      CHECK_THROWS_AS(fut->get(), FutureCanceledError);
      CHECK(out == 0);
    }
    REQUIRE_NOTHROW(fut->release());
    Verify(Method(spy, recycleTimerHandle)).Once();
  }

  SECTION("callLater") {
    Spy(Method(spy, callLater));
    auto fut = loop.callLater(10, [](int in) { return in; }, 3);
    CHECK_FALSE(fut->completed());
    Verify(Method(spy, callLater)).Once();
    REQUIRE_NOTHROW(fut->release());
    Verify(Method(spy, recycleTimerHandle)).Exactly(0);
    lc->runOneIteration();
    Verify(Method(spy, recycleTimerHandle)).Once();
  }

  SECTION("callSoonThreadSafe") {
    LOG_DEBUG("begin callSoonThreadSafe");
    Spy(Method(spy, callSoonThreadSafe));
    TimerFutureThreadSafe<int> *fut =
        loop.callSoonThreadSafe([](int in) { return in; }, 3);
    CHECK_FALSE(fut->completed());
    Verify(Method(spy, callSoonThreadSafe)).Once();

    LOG_DEBUG("before fut->release()");
    REQUIRE_NOTHROW(fut->release());
    Verify(Method(spy, recycleTimerHandle)).Exactly(0);
    lc->runOneIteration();
    Verify(Method(spy, recycleTimerHandle)).Once();
    LOG_DEBUG("end callSoonThreadSafe");
  }
}

} // namespace eventloop_test