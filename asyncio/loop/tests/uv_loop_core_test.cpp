
#include <catch.hpp>
#include <fakeit.hpp>

#include <uv.h>

#include "../loop_core.hpp"

using namespace fakeit;
using namespace asyncio;
namespace uv_loop_core_test {

TEST_CASE("uv_loop_core", "[loop]") {
  uv_loop_t uv_loop;
  REQUIRE(uv_loop_init(&uv_loop) == 0);
  UVLoopCore uvlc(&uv_loop);
  LoopCore *lc = &uvlc;
  void *data = (void *)0xabcd;
  auto callback = [](TimerHandle *h) { h->setData(h); };
  SECTION("finished") {
    TimerHandle *handle;
    SECTION("simple") {
      SECTION("soon") { handle = lc->callSoon(callback, data); }
      SECTION("later") { handle = lc->callLater(10, callback, data); }
      REQUIRE(dynamic_cast<UVTimerHandle *>(handle));
    }
    SECTION("ThreadSafe") {
      handle = lc->callSoonThreadSafe(callback, data);
      REQUIRE(dynamic_cast<UVASyncTimerHandle *>(handle));
    }
    CHECK(handle->data() == data);
    lc->runOneIteration();
    CHECK(handle->data() == handle);
    REQUIRE_THROWS_AS(lc->close(), LoopBusyError);
    REQUIRE_NOTHROW(handle->subRef());
  }
  SECTION("canceled") {
    TimerHandle *handle;
    SECTION("simple") {
      SECTION("soon") { handle = lc->callSoon(callback, data); }
      SECTION("later") { handle = lc->callLater(10, callback, data); }
      REQUIRE(dynamic_cast<UVTimerHandle *>(handle));
    }
    SECTION("ThreadSafe") {
      handle = lc->callSoonThreadSafe(callback, data);
      REQUIRE(dynamic_cast<UVASyncTimerHandle *>(handle));
    }
    CHECK(handle->cancel());
    CHECK(handle->data() == data);
    REQUIRE_THROWS_AS(lc->close(), LoopBusyError);
    REQUIRE_NOTHROW(handle->subRef());
  }
  REQUIRE_NOTHROW(lc->close());
  REQUIRE(uv_loop_close(&uv_loop) == 0);
} // namespace uv_loop_core_test

} // namespace uv_loop_core_test