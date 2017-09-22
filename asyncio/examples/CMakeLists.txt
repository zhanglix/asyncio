add_executable(my_async_sleep my_async_sleep.cpp)
target_link_libraries (my_async_sleep asyncio uv pthread)

add_executable(my_range my_range.cpp)
target_link_libraries (my_range asyncio uv pthread)

add_executable(my_co_range my_co_range.cpp)
target_link_libraries (my_co_range asyncio uv pthread)


add_executable(add_on_loop add_on_loop.cpp)
target_link_libraries (add_on_loop asyncio uv pthread)

add_executable(sleep_sort sleep_sort.cpp)
target_link_libraries (sleep_sort asyncio uv pthread)

add_executable(adds adds.cpp)
target_link_libraries (adds asyncio uv pthread)

add_executable(race race.cpp)
target_link_libraries (race asyncio uv pthread)
