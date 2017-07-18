#include <catch.hpp>
#include <vector>

#include <asyncio/gen.hpp>

using namespace std;
using namespace asyncio;

namespace gen_test {
gen<int> range(int n) {
  for (int i = 0; i < n; i++) {
    co_yield i;
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

TEST_CASE("gen<int> exception thrown") { REQUIRE(false); }
} // namespace gen_test