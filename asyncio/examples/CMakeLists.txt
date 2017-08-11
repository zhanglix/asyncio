add_executable(my_async_sleep my_async_sleep.cpp)
add_executable(my_range my_range.cpp)
add_executable(my_co_range my_co_range.cpp)

add_executable(add_on_loop add_on_loop.cpp)
target_link_libraries (add_on_loop asyncio)

add_executable(sleep_sort sleep_sort.cpp)
target_link_libraries (sleep_sort asyncio)

add_executable(adds adds.cpp)
target_link_libraries (adds asyncio)

add_executable(race race.cpp)
target_link_libraries (race asyncio)
