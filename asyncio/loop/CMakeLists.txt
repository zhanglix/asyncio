add_library(loop 
  loop_core.hpp
  timer_handle.hpp
  uv_loop_core.hpp uv_loop_core.cpp
  uv_timer_handle.hpp uv_timer_handle.cpp

)

add_subdirectory (tests) 
