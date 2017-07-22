#include <catch.hpp>
#include <chrono>
#include <exception>
#include <sstream>
#include <string>

#define ENABLE_ASYNCIO_LOG
#include <asyncio/log.hpp>

#include <asyncio/coro.hpp>
#include <asyncio/utility.hpp>

using namespace std;

USING_ASYNNCIO_NAMESPACE;

class SomeService {
public:
  SomeService() : _callback(nullptr), _userData(nullptr) {}
  typedef void (*Callback)(void *, int status);
  void sendCallbackNow(int expectStatus, void *userData, Callback cb) {
    cb(userData, expectStatus);
  }
  void sendCallbackLater(int expectStatus, void *userData, Callback cb) {
    _callback = cb;
    _userData = userData;
    _status = expectStatus;
  }
  void triggerCallback() {
    if (_callback) {
      _callback(_userData, _status);
    }
  }

private:
  Callback _callback;
  void *_userData;
  int _status;
};

void callback(void *userData, int status) {
  auto awaitablePointer = (AWaitable<int> *)userData;
  if (status < 0) {
    LOG_DEBUG("will raise error: {}", status);
    awaitablePointer->raise(runtime_error("some error"));
  } else {
    LOG_DEBUG("will resume: {}", status);
    awaitablePointer->resume(status);
  }
}

TEST_CASE("AWaitable<int> test with service", "[awaitable]") {
  SomeService service;
  coro<int> co(nullptr);
  SECTION("immediately") {
    auto send = [](SomeService &service, int expectStauts) -> coro<int> {
      AWaitable<int> awaitable;
      service.sendCallbackNow(expectStauts, &awaitable, callback);
      co_return co_await awaitable;
    };
    co = send(service, 3);
  }

  SECTION("later") {
    auto send = [](SomeService &service, int expectStauts) -> coro<int> {
      AWaitable<int> awaitable;
      service.sendCallbackLater(expectStauts, &awaitable, callback);
      co_return co_await awaitable;
    };
    co = send(service, 3);
  }
  co_runner<int> cr(co);
  service.triggerCallback();
  REQUIRE(cr.get_future().get() == 3);
}

TEST_CASE("AWaitable<int> throw", "[awaitable]") {
  LOG_DEBUG("Waitable<int> throw test case started");
  SomeService service;
  coro<int> co(nullptr);
  SECTION("immediately") {
    auto send = [](SomeService &service, int expectStauts) -> coro<int> {
      AWaitable<int> awaitable;
      service.sendCallbackNow(expectStauts, &awaitable, callback);
      co_return co_await awaitable;
    };
    co = send(service, -1);
  }

  SECTION("later") {
    auto send = [](SomeService &service, int expectStauts) -> coro<int> {
      AWaitable<int> awaitable;
      service.sendCallbackLater(expectStauts, &awaitable, callback);
      co_return co_await awaitable;
    };
    co = send(service, -1);
  }
  co_runner<int> cr(co);
  service.triggerCallback();
  REQUIRE_THROWS_AS(cr.get_future().get(), runtime_error);
  LOG_DEBUG("AWaitable<int> throw test case finished!");
}

TEST_CASE("AWaitable<void> unit", "[awaitable]") {
  AWaitable<void> awaitable;
  CHECK_FALSE(awaitable.await_ready());
  CHECK(awaitable.await_suspend(nullptr));
  SECTION("not throw") {
    awaitable.resume();
    CHECK(awaitable.await_ready());
    CHECK_FALSE(awaitable.await_suspend(nullptr));
    awaitable.await_resume();
  }
  SECTION("throw") {
    awaitable.raise(runtime_error("blabla"));
    CHECK(awaitable.await_ready());
    CHECK_FALSE(awaitable.await_suspend(nullptr));
    REQUIRE_THROWS_AS(awaitable.await_resume(), runtime_error);
  }
}

TEST_CASE("co_runner abnormal cases", "[abnormal]") {
  SECTION("nullptr handle") {
    co_runner<void> cr(coro<void>(nullptr));
    REQUIRE_THROWS_AS(cr.get_future().get(), invalid_argument);
  }
}
