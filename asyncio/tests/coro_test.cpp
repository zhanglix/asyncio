#include <catch.hpp>
#include <future>
#include <stdint.h>
#include <utility>

#define ENABLE_ASYNCIO_LOG
#include <asyncio/log.hpp>

#include <asyncio/coro.hpp>
#include <experimental/coroutine>

#include "coro_test_helper.hpp"

using namespace std::experimental;
using namespace asyncio;
using namespace std;

coro<int> foo() {
  LOG_DEBUG("in foo will return 10");
  co_return 10;
}

TEST_CASE("coro_direct_return", "[no_suspening]") {
  auto &&co = foo();
  CHECK_FALSE(co.await_ready());
  CHECK_FALSE(co.await_resume() == 10);
  CHECK_FALSE(co.await_suspend(nullptr));
  CHECK(co.await_resume() == 10);
}

template <typename T> coro<T> goo(handle_leak<T> *leak) {
  LOG_DEBUG("in goo with leak({})", leak->name);
  co_return co_await *leak;
}

template <> coro<void> goo(handle_leak<void> *leak) {
  LOG_DEBUG("in goo with leak({})", leak->name);
  co_await *leak;
  co_return;
}

template <typename T> coro<T> hoo(handle_leak<T> *leak) {
  LOG_DEBUG("in hoo with leak({})", leak->name);
  co_return co_await goo(leak);
}

TEST_CASE("coro_suspended", "[suspended]") {
  LOG_DEBUG("coro_suspended test started");
  handle_leak<int> leak;
  leak.value = 11;
  coro<int> co(nullptr);
  SECTION("goo") { co = goo<int>(&leak); }
  SECTION("hoo") { co = hoo<int>(&leak); }
  LOG_DEBUG("coro created!");
  CHECK_FALSE(leak.handle);
  CHECK_FALSE(co.await_ready());
  CHECK_FALSE(co.await_resume() == leak.value);
  CHECK(co.await_suspend(nullptr));
  LOG_DEBUG("coro suspended. leak.handle: 0x{:x}", (long)leak.handle.address());

  CHECK(leak.handle);
  CHECK_FALSE(leak.handle.done());
  CHECK_FALSE(co.await_resume() == leak.value);
  LOG_DEBUG("leak resume started. leak.handle: 0x{:x}",
            (long)leak.handle.address());

  leak.handle.resume();
  LOG_DEBUG("leak resume ended. leak.handle: 0x{:x}",
            (long)leak.handle.address());

  CHECK(co.await_resume() == leak.value);
}

coro<void> voo() {
  LOG_DEBUG("in voo");
  co_return;
}

TEST_CASE("coro_void", "[void][no_suspening]") {
  LOG_DEBUG("coro_void test started");
  auto &&co = voo();
  CHECK_FALSE(co.await_ready());
  CHECK_FALSE(co.await_suspend(nullptr));
}

TEST_CASE("coro_void_suspended", "[void][suspended]") {
  LOG_DEBUG("coro_void_suspended test started");
  handle_leak<void> leak;
  auto &&co = goo(&leak);
  CHECK_FALSE(co.await_ready());
  CHECK(co.await_suspend(nullptr));
}

template <typename T>
coro<T> joo(handle_leak<T> *leak_1, handle_leak<T> *leak_2,
            handle_leak<void> *leak_void) {
  auto o_1 = co_await goo(leak_1);
  LOG_DEBUG("o_1={}", o_1);
  auto o_2 = co_await foo(); // 10
  LOG_DEBUG("o_2={}", o_2);
  co_await goo(leak_void);
  LOG_DEBUG("after await goo(leak_void)");
  co_return o_1 + o_2 + co_await goo(leak_2);
}

TEST_CASE("coro_hybrid_suspend", "[void][suspended][hybrid]") {
  LOG_DEBUG("coro_hybrid_suspend test started");
  handle_leak<float> leak_1(1.5, "leak_1");
  handle_leak<float> leak_2(100, "leak_2");
  handle_leak<void> leak_void("leak_void");

  auto &&co = joo(&leak_1, &leak_2, &leak_void);
  LOG_DEBUG("co created!");
  co.await_suspend(nullptr);
  LOG_DEBUG("co suspend!");
  leak_1.resume_caller();
  LOG_DEBUG("leak_1 resumed!");
  leak_void.resume_caller();
  LOG_DEBUG("leak_void resumed!");
  leak_2.resume_caller();
  LOG_DEBUG("leak_2 resumed!");
  CHECK(co.await_resume() == 111.5);
}
