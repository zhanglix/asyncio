add_executable(
  utility_test 
  ../../do_test.cpp
  asleep_test.cpp
  )
  
target_link_libraries (utility_test  
  loop 
  uv
  )

ParseAndAddCatchTests(utility_test)