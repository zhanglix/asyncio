#include <catch.hpp>
#include <fakeit.hpp>

#include "../loop_core.hpp"
#include "../timer_handle.hpp"

using namespace asyncio;
using namespace fakeit;
using namespace std;

TEST_CASE("TimerHandle", "[loop]") {
  Mock<LoopCore> mockLoop;
  When(Method(mockLoop, callSoonThreadSafe)).Return(nullptr);
  Fake(Method(mockLoop, recycleTimerHandle));

  LoopCore &loop = mockLoop.get();
  TimerHandle handle(&loop, &mockLoop);
  SECTION("initial state") {
    CHECK(handle.loopCore() == &loop);
    CHECK(handle.data() == (void *)&mockLoop);
    CHECK(handle.refCount() == 1);
  }
  SECTION("addRef") {
    handle.addRef();
    CHECK(handle.refCount() == 2);
    SECTION("subRef") {
      handle.subRef();
      CHECK(handle.refCount() == 1);
    }
  }

  SECTION("subRef to zero") {
    handle.subRef();
    Verify(Method(mockLoop, recycleTimerHandle).Using(&handle)).Once();
  }

  SECTION("subRefThreadSafe") {
    handle.subRefThreadSafe();
    Verify(Method(mockLoop, callSoonThreadSafe)
               .Using(TimerHandle::subRefOnLoop, &handle))
        .Once();
  }

  SECTION("subRefOnLoop") {
    TimerHandle another;
    another.setData(&handle);
    another.addRef(); // 2 ref now
    handle.addRef();  // 2 ref now
    TimerHandle::subRefOnLoop(&another);
    CHECK(handle.refCount() == 1);
    CHECK(another.refCount() == 1);
  }

  SECTION("cancel return true") {
    When(Method(mockLoop, cancelTimer)).Return(true);
    CHECK(handle.cancel());
    Verify(Method(mockLoop, cancelTimer).Using(&handle)).Once();
  }

  SECTION("cancel return false") {
    When(Method(mockLoop, cancelTimer)).Return(false);
    CHECK_FALSE(handle.cancel());
  }
}