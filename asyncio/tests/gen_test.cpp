#include <catch.hpp>
#include <vector>

#include <asyncio/coro.hpp>
#include <asyncio/gen.hpp>
#include <exception>

using namespace std;
using namespace asyncio;

namespace gen_test {
gen<int> range(int n, int throwPos = -1) {
  for (int i = 0; i < n; i++) {
    if (i == throwPos) {
      throw runtime_error("some error!");
    } else {
      co_yield i;
    }
  }
  if (throwPos == n) {
    throw runtime_error("some error!");
  }
}

void do_check(int n) {
  vector<int> expect;
  for (int i = 0; i < n; i++) {
    expect.push_back(i);
  }
  vector<int> actual;
  for (auto &&v : range(n)) {
    actual.push_back(v);
  }
  REQUIRE(expect == actual);
}

TEST_CASE("gen<int> normal", "[range]") {
  SECTION("0") { do_check(0); }
  SECTION("1") { do_check(1); }
  SECTION("2") { do_check(2); }
  SECTION("3") { do_check(3); }
}

void traverse(int pos, int size) {
  for (auto &&v : range(size, pos)) {
    (void)v;
  }
}
void do_check_exception(int pos, int size = 3) {
  REQUIRE_THROWS_AS(traverse(pos, size), runtime_error);
}

TEST_CASE("gen<int> exception thrown") {
  SECTION("0") { do_check_exception(0); }
  SECTION("1") { do_check_exception(1); }
  SECTION("2") { do_check_exception(2); }
  SECTION("3") { do_check_exception(3); }
}

// @todo move the following two lines in a separate test framework to make
// sure it will call the comiler to generate compiling error!
// coro<int> foo(int n) { co_return n; }
// gen<int> will_generate_compiling_error(int n) { co_yield co_await foo(n); }
} // namespace gen_test