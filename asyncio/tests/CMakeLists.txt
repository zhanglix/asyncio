add_executable(
  asyncio_test 
  do_test.cpp
  coro_test.cpp)
#target_link_libraries (asyncio_test  asyncio)

add_executable(
  co_drafts  co_drafts.cpp
)
ParseAndAddCatchTests(asyncio_test)