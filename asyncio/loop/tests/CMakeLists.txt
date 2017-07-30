add_executable(
  loop_test 
  ../../do_test.cpp
  timer_handle_test.cpp  
  parameter_pack_test.cpp
  event_loop_test.cpp
  trivial_loop.hpp trivial_loop.cpp
  spy_test.cpp
  )
  
target_link_libraries (loop_test  loop)

ParseAndAddCatchTests(loop_test)