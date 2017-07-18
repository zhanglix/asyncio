add_executable(
  asyncio_test 
  do_test.cpp
  coro_test.cpp
  std_promise_test.cpp
  co_utility_test.cpp
  )
#target_link_libraries (asyncio_test  asyncio)

add_executable(
  co_drafts  co_drafts.cpp
)
# working command on linux
# clang++ -I/usr/local/include/c++/v1 -std=c++14 -stdlib=libc++ -fcoroutines-ts -o co_drafts co_drafts.cpp  -lc++abi
# automate build on linux later
ParseAndAddCatchTests(asyncio_test)