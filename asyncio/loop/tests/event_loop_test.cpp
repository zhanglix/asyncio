#include <catch.hpp>
#include <fakeit.hpp>

#include "../event_loop.hpp"
#include "../loop_core.hpp"
#include "../loop_exception.hpp"
#include "trivial_loop.hpp"

using namespace std;
using namespace fakeit;
using namespace asyncio;

namespace eventloop_test {

TEST_CASE("eventloop callsoon", "[loop]") {
  TrivialLoop trivialLoop;
  Mock<LoopCore> spy(trivialLoop);
  LoopCore *lc = &spy.get();
  EventLoop loop(lc);

  SECTION("int") {
    auto fut = loop.callSoon([](int in) { return in; }, 3);
    REQUIRE_FALSE(fut->completed());
    SECTION("done") {
      lc->runOneIteration();
      CHECK(fut->completed());
      CHECK(fut->get() == 3);
      CHECK_FALSE(fut->cancel());
    }
    SECTION("canceled") {
      CHECK(fut->cancel());
      CHECK(fut->completed());
      CHECK_THROWS_AS(fut->get(), FutureCanceledError);
    }
  }

  SECTION("void") {
    int out = 0;
    auto fut = loop.callSoon([](int in, int &out) { out = in; }, 3, ref(out));
    CHECK_FALSE(fut->completed());
    SECTION("done") {
      lc->runOneIteration();
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
  }
}
} // namespace eventloop_test