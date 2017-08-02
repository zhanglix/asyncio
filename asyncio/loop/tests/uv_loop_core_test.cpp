
#include <catch.hpp>
#include <fakeit.hpp>

#include <uv.h>

#include "../loop_exception.hpp"
#include "../uv_loop_core.hpp"
#include "../uv_timer_handle.hpp"

#define ENABLE_ASYNCIO_LOG
#include <asyncio/log.hpp>

using namespace fakeit;
using namespace asyncio;
namespace uv_loop_core_test {

TEST_CASE("uv_loop_core", "[loop]") {
  UVLoopCore uvlc;
  uv_loop_t *uv_loop = uvlc.getUVLoop();

  LoopCore *lc = &uvlc;
  void *data = (void *)0xabcd;
  auto callback = [](TimerHandle *h) { h->setData(h); };
  SECTION("finished") {
    LOG_DEBUG("begin finished!");
    TimerHandle *handle;
    SECTION("simple") {
      SECTION("soon") {
        LOG_DEBUG("begin simple.soon!");
        handle = lc->callSoon(callback, data);
      }
      SECTION("later") {
        LOG_DEBUG("begin simple.later!");
        handle = lc->callLater(10, callback, data);
      }
      REQUIRE(dynamic_cast<UVTimerHandle *>(handle));
    }
    SECTION("ThreadSafe") {
      LOG_DEBUG("begin ThreadSafe!");
      handle = lc->callSoonThreadSafe(callback, data);
      REQUIRE(dynamic_cast<UVASyncTimerHandle *>(handle));
    }
    CHECK(handle->data() == data);
    lc->runOneIteration();
    CHECK(handle->data() == handle);
    REQUIRE_THROWS_AS(lc->close(), LoopBusyError);
    CHECK(lc->activeHandlesCount() == 1);
    REQUIRE_NOTHROW(handle->subRef());
    CHECK(lc->activeHandlesCount() == 0);

    LOG_DEBUG("end finished!");
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
    CHECK(lc->activeHandlesCount() == 1);
    REQUIRE_NOTHROW(handle->subRef());
    CHECK(lc->activeHandlesCount() == 0);
  }
  REQUIRE_NOTHROW(lc->close());
}

} // namespace uv_loop_core_test