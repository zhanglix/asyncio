#include <catch.hpp>

#include <functional>
#include <future>
#include <sstream>
#include <tuple>
#define ENABLE_ASYNCIO_LOG

#include <asyncio/common.hpp>
#include <asyncio/loop.hpp>

#include "../sleep.hpp"
#include "../wait.hpp"

using namespace std;

BEGIN_ASYNCIO_NAMESPACE;

namespace wait_test {
TEST_CASE("tuple", "[tuple]") {
  SECTION("promise and future", "bla bla") {
    auto promises = make_tuple(std::promise<int>());
    auto futures = get<0>(promises).get_future();
    get<0>(promises).set_value(10);
    REQUIRE(futures.get() == 10);
  }

  SECTION("equal") {
    auto lhs = make_tuple(1u, 2u, 3u);
    auto rhs = make_tuple(1, 2, 3);
    CHECK(lhs == rhs);
  }
}

coro<int> sleepReturn(EventLoop *loop, int v, uint64_t ms, string &out) {
  co_await sleep(loop, ms);
  stringstream ss;
  ss << v;
  out += ss.str();
  co_return v;
}

TEST_CASE("wait", "[wait]") {
  EventLoop loop;
  string out;
  function<coro<void>()> aTest;
  Future<int> *fut0;
  Future<int> *fut1;
  fut0 = loop.callLater(2, [&] { return 1; });
  fut1 = loop.callSoon([&] { return 2; });
  vector<FutureBase *> futs{fut0, fut1};

  SECTION("futures") {
    SECTION("allFutures") {
      SECTION("variable args") {
        aTest = [&]() -> coro<void> {
          co_await allFutures(fut0, fut1);
          CHECK(fut0->done());
          CHECK(fut1->done());
        };
      }
      SECTION("vector arg") {
        aTest = [&]() -> coro<void> {
          co_await allFutures(futs);
          CHECK(fut0->done());
          CHECK(fut1->done());
        };
      }
    }

    SECTION("anyFuture") {
      SECTION("all waited") {
        aTest = [&]() -> coro<void> {
          FutureCoGen anyWaiter(futs);
          auto iter = co_await anyWaiter.begin();
          CHECK(iter != anyWaiter.end());
          CHECK(*iter == fut1);
          CHECK((*iter)->done());
          CHECK_FALSE(fut0->done());
          co_await++ iter;
          CHECK(*iter == fut0);
          CHECK(fut0->done());
          co_await++ iter;
          CHECK_FALSE(iter != anyWaiter.end());
        };
      }
      SECTION("all waited in for") {
        aTest = [&]() -> coro<void> {
          size_t i = 0;
          for
            co_await(auto &&fut : FutureCoGen(futs)) {
              if (i == 0) {
                CHECK(fut == fut1);
              } else {
                CHECK(fut == fut0);
              }
              CHECK(fut->done());
              i++;
            }
        };
      }
      SECTION("wait any future part wait") {
        aTest = [&]() -> coro<void> {
          FutureCoGen anyWaiter(futs);
          auto iter = co_await anyWaiter.begin();
          CHECK(iter != anyWaiter.end());
          CHECK(*iter == fut1);
          CHECK((*iter)->done());
          CHECK_FALSE(fut0->done());
        };
      }
    }
  }

  SECTION("all") {
    aTest = [&]() -> coro<void> {
      auto results = co_await all(&loop, sleepReturn(&loop, 1, 10, out),
                                  sleepReturn(&loop, 2, 5, out),
                                  sleepReturn(&loop, 3, 0, out));
      CHECK(results == make_tuple(1, 2, 3));
      CHECK(out == "321");
    };
  }

  auto task = loop.createTask(aTest());
  loop.runUntilDone(task);
  task->release();
  loop.runUntilDone(fut0);
  fut0->release();
  fut1->release();
}
} // namespace wait_test

END_ASYNCIO_NAMESPACE;