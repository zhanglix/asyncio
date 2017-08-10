add_executable(
  utility_test 
  ../../do_test.cpp
  sleep_test.cpp
  wait_test.cpp
  )
  
target_link_libraries (utility_test  
  utility
  loop 
  uv
  )

ParseAndAddCatchTests(utility_test)