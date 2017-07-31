
#include <catch.hpp>
#include <fakeit.hpp>

#include <uv.h>

#include "../loop_core.hpp"

using namespace fakeit;
using namespace asyncio;
namespace uv_loop_core_test {

TEST_CASE("uv_loop ", "[uv]") {
  uv_loop_t uv_loop;
  REQUIRE(uv_loop_init(&uv_loop) == 0);
  int timeout;
  uint64_t t0, t1;
  SECTION("no handles") {
    timeout = uv_backend_timeout(&uv_loop);
    CHECK(timeout == 0);
    t0 = uv_now(&uv_loop);
    REQUIRE(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
    t1 = uv_now(&uv_loop);
    // CHECK(t1 - t0 == timeout);
  }

  SECTION("async") {
    uv_async_t uv_async;
    auto callback = [](uv_async_t *handle) { handle->data = handle; };
    REQUIRE(uv_async_init(&uv_loop, &uv_async, callback) == 0);
    timeout = uv_backend_timeout(&uv_loop);
    REQUIRE(timeout == -1);
    REQUIRE(uv_async_send(&uv_async) == 0);
    t0 = uv_now(&uv_loop);
    CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 1);
    t1 = uv_now(&uv_loop);
    //    CHECK(t1 - t0 > 0);
    CHECK(uv_async.data == &uv_async);
    uv_close((uv_handle_t *)(&uv_async), nullptr);
    CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
  }

  SECTION("timer") { CHECK(false); }

  REQUIRE(uv_loop_close(&uv_loop) == 0);
}

TEST_CASE("uv_loop_core", "[loop]") {

  // UVLoopCore uvlc(&uv_loop);
  // LoopCore *lc = &uvlc;
  // TimerHandle *handleReceived = nullptr;
  // auto callback = [](TimerHandle *h) {
  //   auto d = (void **)(h->data());
  //   *d = h;
  // };
  // SECTION("callSoon") {
  //   TimerHandle *handle = lc->callSoon(callback, &handleReceived);
  //   REQUIRE(handleReceived == nullptr);
  //   lc->runOneIteration();
  //   CHECK(handle == handleReceived);
  //   REQUIRE_NOTHROW(handle->subRef());
  // }
}

} // namespace uv_loop_core_test