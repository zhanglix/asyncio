add_executable(
  coro_test 
  ../../do_test.cpp
  coro_test.cpp
  std_promise_test.cpp
  utility_test.cpp
  gen_test.cpp
  co_gen_test.cpp
  allocator_test.cpp
  )
target_link_libraries (coro_test  asyncio pthread)

add_executable(
  co_drafts  co_drafts.cpp
)
# working command on linux
# clang++ -I/usr/local/include/c++/v1 -std=c++14 -stdlib=libc++ -fcoroutines-ts -o co_drafts co_drafts.cpp  -lc++abi
# automate build on linux later
ParseAndAddCatchTests(coro_test)