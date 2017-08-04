#include "catch.hpp"

#include <string>

#include "../asleep.hpp"

using namespace std;

USING_ASYNNCIO_NAMESPACE;
namespace asleep_test {
TEST_CASE("asleep", "[utility]") {
  EventLoop loop;
  string output;
  auto foo = [&](uint64_t ms, string word) -> coro<void> {
    co_await asleep(&loop, ms);
    output += word;
  };
  auto first = loop.createTask(foo(30, "A"));
  loop.createTask(foo(20, "B"))->release();
  loop.createTask(foo(10, "C"))->release();
  loop.runUntilComplete(first);
  first->release();
  CHECK(output == "CBA");
}
} // namespace asleep_test