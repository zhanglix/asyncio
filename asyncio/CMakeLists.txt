add_subdirectory (coro) 
add_subdirectory(loop)
add_subdirectory(utility)
add_subdirectory(examples)

add_library(asyncio SHARED $<TARGET_OBJECTS:loop> $<TARGET_OBJECTS:utility>)
target_link_libraries (asyncio uv )
add_library(asyncio_static STATIC $<TARGET_OBJECTS:loop> $<TARGET_OBJECTS:utility>)





