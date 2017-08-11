add_library(utility OBJECT
  loop_awaitable.hpp
  sleep.hpp sleep.cpp
)

add_subdirectory (tests) 
