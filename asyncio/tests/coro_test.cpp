#include <catch.hpp>
#include <future>
#include <stdint.h>
#include <utility>

#define ENABLE_ASYNCIO_LOG

#include <asyncio/coro.hpp>
#include <experimental/coroutine>

#include "coro_test_helper.hpp"

using namespace std::experimental;
using namespace asyncio;
using namespace std;

coro<int> foo() { co_return 10; }

TEST_CASE("coro_direct_return", "[coro]") {
  auto &&co = foo();
  CHECK_FALSE(co.await_ready());
  CHECK_FALSE(co.await_resume() == 10);
  CHECK_FALSE(co.await_suspend(nullptr));
  CHECK(co.await_resume() == 10);
}

template <typename T> coro<T> goo(handle_leak<T> *leak) {
  auto result = co_await * leak;
  co_return result;
}

template <typename T> coro<T> hoo(handle_leak<T> *leak) {
  co_return co_await goo(leak);
}

TEST_CASE("coro_suspended", "[coro]") {
  handle_leak<int> leak;
  leak.value = 11;
  coro<int> co(nullptr);
  // SECTION("goo") { co = goo<int>(&leak); }
  SECTION("hoo") { co = hoo<int>(&leak); }
  CAPTURE(leak.handle.address());
  CHECK_FALSE(leak.handle);
  CHECK_FALSE(co.await_ready());
  CHECK_FALSE(co.await_resume() == leak.value);
  CHECK(co.await_suspend(nullptr));
  CAPTURE(leak.handle.address());

  CHECK(leak.handle);
  CHECK_FALSE(leak.handle.done());
  CHECK_FALSE(co.await_resume() == leak.value);
  leak.handle.resume();
  CHECK(co.await_resume() == leak.value);
  CHECK(leak.handle.done());
}
