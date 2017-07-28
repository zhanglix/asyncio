add_executable(
  loop_test 
  ../../do_test.cpp
  timer_handle_test.cpp  
  )
  
target_link_libraries (loop_test  loop)

ParseAndAddCatchTests(loop_test)