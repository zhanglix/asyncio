add_library(loop OBJECT
  loop_core.hpp
  handle_base.hpp handle_base.cpp
  timer_handle.hpp timer_handle.cpp
  uv_loop_core.hpp uv_loop_core.cpp
  uv_handle.hpp uv_handle.cpp
  uv_service.hpp uv_service.cpp
  uv_async_service.hpp uv_async_service.cpp
  uv_timer_service.hpp uv_timer_service.cpp
  event_loop.hpp event_loop.cpp
)

add_subdirectory (tests) 
