
#include <catch.hpp>
#include <fakeit.hpp>

#include <uv.h>

#define ENABLE_ASYNCIO_LOG
#include <asyncio/log.hpp>

using namespace fakeit;
namespace uv_test {

TEST_CASE("uv_loop", "[uv]") {
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

  SECTION("timer") {
    uv_timer_t uv_timer;
    auto callback = [](uv_timer_t *handle) { handle->data = handle; };
    REQUIRE(uv_timer_init(&uv_loop, &uv_timer) == 0);
    SECTION("not started") {
      timeout = uv_backend_timeout(&uv_loop);
      REQUIRE(timeout == 0);
      CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
    }
    SECTION("start soon") {
      CHECK(!uv_timer_start(&uv_timer, callback, 0, 0));
      CHECK(uv_backend_timeout(&uv_loop) == 0);
      CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
      CHECK(uv_timer.data == &uv_timer);
    }
    SECTION("start later") {
      CHECK(!uv_timer_start(&uv_timer, callback, 10, 0));
      CHECK(uv_backend_timeout(&uv_loop) == 10);
      CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
      CHECK(uv_timer.data == &uv_timer);
    }

    SECTION("start two timer later") {
      uv_timer_t uv_timer2;
      REQUIRE(uv_timer_init(&uv_loop, &uv_timer2) == 0);

      CHECK(!uv_timer_start(&uv_timer, callback, 10, 0));
      CHECK(!uv_timer_start(&uv_timer2, callback, 50, 0));
      CHECK(uv_backend_timeout(&uv_loop) == 10);
      CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 1);
      CHECK(uv_timer.data == &uv_timer);
      CHECK(uv_timer2.data != &uv_timer2);
      CHECK(uv_backend_timeout(&uv_loop) > 0);
      CHECK(uv_backend_timeout(&uv_loop) <= 40);
      CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
      CHECK(uv_timer2.data == &uv_timer2);
      uv_close((uv_handle_t *)(&uv_timer2), nullptr);
    }

    uv_close((uv_handle_t *)(&uv_timer), nullptr);
    CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
  }

  SECTION("hybrid") {
    uv_async_t uv_async;
    auto async_cb = [](uv_async_t *handle) { handle->data = handle; };
    REQUIRE(uv_async_init(&uv_loop, &uv_async, async_cb) == 0);
    uv_unref((uv_handle_t *)&uv_async);

    uv_timer_t uv_timer;
    auto timer_cb = [](uv_timer_t *handle) { handle->data = handle; };
    REQUIRE(uv_timer_init(&uv_loop, &uv_timer) == 0);
    CHECK(!uv_timer_start(&uv_timer, timer_cb, 0, 0));

    LOG_DEBUG("Before First uv_run!");
    CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
    CHECK(uv_timer.data == &uv_timer);

    REQUIRE(uv_async_send(&uv_async) == 0);
    uv_ref((uv_handle_t *)&uv_async);

    LOG_DEBUG("Before second uv_run!");
    CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 1);

    uv_close((uv_handle_t *)(&uv_timer), nullptr);
    uv_close((uv_handle_t *)(&uv_async), nullptr);

    LOG_DEBUG("Before third uv_run!");
    CHECK(uv_run(&uv_loop, UV_RUN_ONCE) == 0);
  }
  REQUIRE(uv_loop_close(&uv_loop) == 0);
}

} // namespace uv_test