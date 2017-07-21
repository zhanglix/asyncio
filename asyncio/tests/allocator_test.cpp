#include <catch.hpp>
#include <cstddef>
#include <set>

#define ENABLE_ASYNCIO_LOG
#include <asyncio/log.hpp>

#include <asyncio/co_gen.hpp>
#include <asyncio/coro.hpp>
#include <asyncio/gen.hpp>
#include <asyncio/utility.hpp>

using namespace std;
using namespace asyncio;

namespace allocator_test {
struct Arena {
  void *allocate(size_t size) noexcept {
    size_t newSize = sizeof(void *) + size;
    void *memory = malloc(newSize);

    return dataAddress(memory);
  }
  void *dataAddress(void *memory) noexcept {
    void **p = (void **)memory;
    p[0] = this;
    void *ret = (void *)(p + 1);
    try {
      addressAllocated.insert(ret);
      LOG_DEBUG("memory:{}, data:{}, this:{}, p[0]:{}", memory, ret,
                (void *)this, p[0]);
    } catch (...) {
    }

    return ret;
  }

  static Arena &getDefaultArena() {
    static Arena arena;
    return arena;
  }

  void *memoryAddress(void *dataAddress) noexcept {
    void **p = (void **)dataAddress;
    return (void *)(p - 1);
  }

  static Arena *getArena(void *dataAddress) noexcept {
    void **p = (void **)dataAddress;
    return (Arena *)(p[-1]);
  }

  void free(void *dataAddress) noexcept {
    void *memory = memoryAddress(dataAddress);
    ::free(memory);
    addressAllocated.erase(dataAddress);
    LOG_DEBUG("memory:{}, date:{}, this:{}", memory, dataAddress, (void *)this);
  }
  static void deallocate(void *p) noexcept {
    Arena *arena = getArena(p);
    arena->free(p);
  }
  set<void *> addressAllocated;
};

TEST_CASE("Arena allocator", "[allcoator]") {
  Arena pool;
  void *p = pool.allocate(10);
  REQUIRE(pool.addressAllocated.find(p) != pool.addressAllocated.end());
  Arena::deallocate(p);
  REQUIRE(pool.addressAllocated.find(p) == pool.addressAllocated.end());
}

struct Allocator {
  template <typename... Args>
  void *operator new(size_t size, Arena &arena, Args const &...) noexcept {
    void *p = arena.allocate(size);
    LOG_DEBUG("Allocated memory:{}", p);
    return p;
  }

  void *operator new(size_t size) noexcept {
    void *p = Arena::getDefaultArena().allocate(size);
    LOG_DEBUG("Allocated memory :{}", p);
    return p;
  }

  void operator delete(void *p, size_t size) noexcept {
    Arena::deallocate(p);
    LOG_DEBUG("Freed memory:{}", p);
  }
};

coro<int, Allocator> coo(int x) { co_return x; }
gen<int, Allocator> range(int n) {
  for (int i = 0; i < n; i++) {
    co_yield i;
  }
}

co_gen<int, Allocator> co_range(int n) {
  for (int i = 0; i < n; i++) {
    co_yield coo(i);
  }
}

TEST_CASE("coroutines with default arena", "[allcoator]") {
  Arena &arena = Arena::getDefaultArena();
  SECTION("coro") {
    auto co = coo(3);
    REQUIRE(arena.addressAllocated.size() == 1);
    REQUIRE(arena.addressAllocated.find(co.handle_address()) !=
            arena.addressAllocated.end());
    co_runner<int, decltype(co)> cr(co);
    REQUIRE(cr.get_future().get() == 3);
  }

  SECTION("gen") {
    auto &&g = range(5);
    REQUIRE(arena.addressAllocated.size() == 1);
    REQUIRE(arena.addressAllocated.find(g.handle_address()) !=
            arena.addressAllocated.end());
    int sum = 0;
    for (auto &&v : g) {
      sum += v;
    }
    REQUIRE(sum == 10);
  }

  SECTION("co_gen") {
    auto &&co_g = co_range(4);
    REQUIRE(arena.addressAllocated.size() == 1);
    REQUIRE(arena.addressAllocated.find(co_g.handle_address()) !=
            arena.addressAllocated.end());
    auto &&co_begin = co_g.begin();

    REQUIRE(arena.addressAllocated.size() == 2);
    REQUIRE(arena.addressAllocated.find(co_begin.handle_address()) !=
            arena.addressAllocated.end());

    co_begin.await_suspend(nullptr);
    auto &&iter = co_begin.await_resume();
    REQUIRE(*iter == 0);
    auto &&co_iter = ++iter;

    REQUIRE(arena.addressAllocated.size() == 3);
    REQUIRE(arena.addressAllocated.find(co_iter.handle_address()) !=
            arena.addressAllocated.end());
  }

  SECTION("co_runner void") {
    int in = 3, out = 4;
    auto foo = [](int input, int &output) -> coro<void, Allocator> {
      output = input;
      co_return;
    };
    co_runner<void, coro<void, Allocator>> cr(foo(in, out));
    cr.get_future().get();
    REQUIRE(in == out);
  }

  REQUIRE(arena.addressAllocated.size() == 0);
}

coro<int, Allocator> foo(Arena &arena, int x) { co_return x; }

TEST_CASE("coro with specific arena", "[allcoator][!shouldfail]") {
  Arena arena;
  SECTION("using a arena") {
    auto co = foo(arena, 3);
    REQUIRE(arena.addressAllocated.size() == 1);
    co_runner<int, decltype(co)> cr(co);
    REQUIRE(cr.get_future().get() == 3);
  }
  REQUIRE(arena.addressAllocated.size() == 0);
}
} // namespace allocator_test