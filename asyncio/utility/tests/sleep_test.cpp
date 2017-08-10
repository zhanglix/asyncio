#include "catch.hpp"

#include <string>

#include "../sleep.hpp"

using namespace std;

USING_ASYNNCIO_NAMESPACE;
namespace sleep_test {
TEST_CASE("sleep", "[utility]") {
  EventLoop loop;
  string output;
  auto foo = [&](uint64_t ms, string word) -> coro<void> {
    co_await sleep(&loop, ms);
    output += word;
  };
  auto first = loop.createTask(foo(30, "A"));
  loop.createTask(foo(20, "B"))->release();
  loop.createTask(foo(10, "C"))->release();
  loop.runUntilDone(first);
  first->release();
  CHECK(output == "CBA");
}
} // namespace sleep_test