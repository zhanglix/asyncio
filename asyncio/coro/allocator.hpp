#pragma once

#include <asyncio/common.hpp>

BEGIN_ASYNCIO_NAMESPACE;
class DefaultAllocator {
public:
  // you can supply a customized allocator to your coro<>, gen<>, co_gen<>
  // template  to improve efficiency for memory allocation for the state of
  // coroutine.

  // eg:

  // using customized method for all coroutine
  // static void *operator new(size_t size) noexcept {
  //   void *p = malloc(size);
  //   cout << "Allocated memory: " << p << " size: " << size << endl;
  //   return p;
  // }

  // or using a customized pool for differenct coroutine
  // NB: This seems not working now with llvm 5.0
  // template <typename... Args>
  // static void *operator new(size_t size, Arena& pool, Args const &...)
  // noexcept {
  //   void *p = pool.allocate(size);
  //   cout << "Allocated memory: " << p << " size: " << size << endl;
  //   return p;
  // }

  // and you need
  // static void operator delete(void *p, size_t size) noexcept {
  //   free(p);
  //   cout << "Freed memory: " << p << " size: " << size << endl;
  // }
};
END_ASYNCIO_NAMESPACE;