#include <catch.hpp>
#include <future>
#include <stdint.h>

#include <asyncio/coro.hpp>
#include <experimental/coroutine>

using namespace std::experimental;
using namespace asyncio;
using namespace std;

coro<int> foo() { co_return 10; }

struct handle_hacker {
  handle_hacker(coroutine_handle<> *h) : _h(h) {}
  bool await_ready() const noexcept { return false; }
  void await_suspend(coroutine_handle<> h) const noexcept { *_h = h; }
  int await_resume() const noexcept { return 11; }
  coroutine_handle<> *_h;
};

coro<int> goo(coroutine_handle<> *h) {
  handle_hacker hh(h);
  auto result = co_await hh;
  co_return result;
}

TEST_CASE("coro", "[async]") {
  auto &&co = foo();
  CHECK_FALSE(co.await_ready());
  CHECK_FALSE(co.await_resume() == 10);
  CHECK_FALSE(co.await_suspend(nullptr));
  CHECK(co.await_resume() == 10);
}

TEST_CASE("coroFutureSet", "[async]") {
  coroutine_handle<> h;
  auto &&co = goo(&h);
  CHECK_FALSE(h);
  CHECK_FALSE(co.await_ready());
  CHECK_FALSE(co.await_resume() == 11);
  CHECK(co.await_suspend(nullptr));
  CHECK(h);
  CHECK_FALSE(h.done());
  CHECK_FALSE(co.await_resume() == 11);
  h.resume();
  CHECK(co.await_resume() == 11);
  CHECK(h.done());
}
