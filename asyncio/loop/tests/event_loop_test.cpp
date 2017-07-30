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

TEST_CASE("eventloop timer", "[loop]") {
  TrivialLoop trivialLoop;
  Mock<LoopCore> spy(trivialLoop);
  Spy(Method(spy, callSoon));

  LoopCore *lc = &spy.get();
  EventLoop loop(lc);

  SECTION("int") {
    auto fut = loop.callSoon([](int in) { return in; }, 3);
    CHECK_FALSE(fut->completed());
    Verify(Method(spy, callSoon)).Once();
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
    REQUIRE_NOTHROW(fut->release());
  }

  SECTION("void") {
    int out = 0;
    auto fut = loop.callSoon([](int in, int &out) { out = in; }, 3, ref(out));
    CHECK_FALSE(fut->completed());
    Verify(Method(spy, callSoon)).Once();

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
    REQUIRE_NOTHROW(fut->release());
  }

  SECTION("callLater") {
    Spy(Method(spy, callLater));
    auto fut = loop.callLater(10, [](int in) { return in; }, 3);
    CHECK_FALSE(fut->completed());
    Verify(Method(spy, callLater)).Once();
    REQUIRE_NOTHROW(fut->release());
  }

  SECTION("callSoonThreadSafe") {
    Spy(Method(spy, callSoonThreadSafe));
    auto fut = loop.callSoonThreadSafe([](int in) { return in; }, 3);
    CHECK_FALSE(fut->completed());
    Verify(Method(spy, callSoonThreadSafe)).Once();
    REQUIRE_NOTHROW(fut->release());
    Verify(Method(spy, callSoonThreadSafe)).Once();
  }
}
} // namespace eventloop_test