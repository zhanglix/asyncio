#include <catch.hpp>

#include "../extended_queue.hpp"

using namespace std;
using namespace asyncio;

namespace extended_queue_test {
TEST_CASE("extended_queue", "[queue]") {
  ExtendedQueue<int> queue;
  SECTION("empty") {
    CHECK(queue.empty());
    CHECK_FALSE(queue.erase(3));
  }
  queue.push(0);
  SECTION("one") {
    CHECK_FALSE(queue.erase(1));
    CHECK_FALSE(queue.empty());
    CHECK(queue.front() == 0);
    SECTION("erase") { CHECK(queue.erase(0)); }
    SECTION("pop") { queue.pop(); }
    CHECK(queue.empty());
  }
  queue.push(1);
  SECTION("two") {
    CHECK_FALSE(queue.erase(2));
    CHECK_FALSE(queue.empty());
    CHECK(queue.front() == 0);
    SECTION("erase first") {
      CHECK(queue.erase(0));
      CHECK(queue.front() == 1);
    }
    SECTION("erase second") {
      CHECK(queue.erase(1));
      CHECK(queue.front() == 0);
    }
    SECTION("pop") {
      queue.pop();
      CHECK(queue.front() == 1);
    }
    CHECK(queue.size() == 1);
  }
}
} // namespace extended_queue_test