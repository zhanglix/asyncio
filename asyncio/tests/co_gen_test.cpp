#include <catch.hpp>
#include <exception>
#include <vector>

#define ENABLE_ASYNCIO_LOG
#include <asyncio/log.hpp>
#include <vector>

#include <asyncio/co_gen.hpp>
#include <asyncio/coro.hpp>
#include <asyncio/utility.hpp>

using namespace std;
using namespace asyncio;

namespace co_gen_test {

static vector<AWaitable<int>> awaitables;
coro<int> identity_no_suspend(int n) {
  LOG_DEBUG("identity_no_suspend{}", n);
  co_return n;
}
coro<int> awaitable_suspended(int i) {
  LOG_DEBUG("awaitable_suspended");
  co_return co_await awaitables[i];
}

co_gen<int> range(int n, bool wait, int throwPos = -1) {
  for (int i = 0; i < n; i++) {
    if (i == throwPos) {
      throw runtime_error("some error!");
    } else {
      co_yield wait ? awaitable_suspended(i) : identity_no_suspend(i);
    }
  }
  if (throwPos == n) {
    throw runtime_error("some error!");
  }
}

coro<void> co_check(int n, bool wait) {
  LOG_DEBUG("begin co_check");
  vector<int> expect;
  for (int i = 0; i < n; i++) {
    expect.push_back(i);
  }
  vector<int> actual;
  LOG_DEBUG("before for co_await()");
  for
    co_await(auto &&v : range(n, wait)) {
      LOG_DEBUG("co_check got {}", v);
      actual.push_back(v);
    }
  REQUIRE(expect == actual);
}

void do_check(int n, bool wait) {
  awaitables.clear();
  awaitables.resize(n);
  co_runner<void> cr(co_check(n, wait));
  if (wait) {
    for (int i = 0; i < n; i++) {
      LOG_DEBUG("awaitable.resume({});", i);
      awaitables[i].resume(i);
    }
  }
  cr.get_future().get();
}

TEST_CASE("co_gen<int> normal", "[range]") {
  SECTION("0, false") { do_check(0, false); }
  SECTION("0, true") { do_check(0, true); }
  SECTION("1, false") { do_check(1, false); }
  SECTION("1, true") { do_check(1, true); }
  SECTION("2, false") { do_check(2, false); }
  SECTION("2, true") { do_check(2, true); }
  SECTION("3, false") { do_check(3, false); }
  SECTION("3, true") { do_check(3, true); }
}

coro<void> traverse(int pos, bool wait, int size) {
  for
    co_await(auto &&v : range(size, wait, pos)) { (void)v; }
}

void do_check_exception(int pos, bool wait = false, int size = 3) {
  awaitables.clear();
  awaitables.resize(size);
  co_runner<void> cr(traverse(pos, wait, size));
  if (wait) {
    for (int i = 0; i < pos; i++) {
      LOG_DEBUG("awaitable.resume({});", i);
      awaitables[i].resume(i);
    }
  }
  REQUIRE_THROWS_AS(cr.get_future().get(), runtime_error);
}

TEST_CASE("co_gen<int> exception thrown") {
  SECTION("0") { do_check_exception(0); }
  SECTION("1") { do_check_exception(1); }
  SECTION("2") { do_check_exception(2); }
  SECTION("3") { do_check_exception(3); }
}
TEST_CASE("co_gen<int> exception thrown wait") {
  SECTION("0") { do_check_exception(0, true); }
  SECTION("1") { do_check_exception(1, true); }
  SECTION("2") { do_check_exception(2, true); }
  SECTION("3") { do_check_exception(3, true); }
}

// @todo move the following two lines in a separate test framework to make
// sure it will call the comiler to generate compiling error!
// coro<int> foo(int n) { co_return n; }
// co_gen<int> will_generate_compiling_error(int n) { co_yield co_await foo(n);
// }
} // namespace co_gen_test