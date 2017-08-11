add_library(utility OBJECT
  loop_awaitable.hpp
  sleep.hpp sleep.cpp
  wait.hpp wait.cpp
)

add_subdirectory (tests) 
