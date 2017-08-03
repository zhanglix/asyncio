
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
    LOG_DEBUG("before runOneIteration()");
    lc->runOneIteration();
    CHECK(handle->data() == handle);
    CHECK(handle->completed());
    CHECK_FALSE(handle->cancel());
    LOG_DEBUG("before close() exepct throw");
    REQUIRE_THROWS_AS(lc->close(), LoopBusyError);
    CHECK(lc->activeHandlesCount() == 1);
    CHECK(handle->subRef() == 0);
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
    lc->runOneIteration();
    CHECK(handle->data() == data);
    REQUIRE_THROWS_AS(lc->close(), LoopBusyError);
    CHECK(lc->activeHandlesCount() == 1);
    REQUIRE_NOTHROW(handle->subRef());
    CHECK(lc->activeHandlesCount() == 0);
  }
  LOG_DEBUG("before close() ending");

  REQUIRE_NOTHROW(lc->close());
}

TEST_CASE("uv_loop_core_not_owner", "[loop]") {
  uv_loop_t uv_loop;
  REQUIRE(!uv_loop_init(&uv_loop));
  UVLoopCore uvlc(&uv_loop);
  LoopCore *lc = &uvlc;
  REQUIRE_NOTHROW(lc->close());
  REQUIRE(!uv_loop_close(&uv_loop));
}

} // namespace uv_loop_core_test