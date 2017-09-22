add_executable(
  loop_test 
  ../../do_test.cpp
  parameter_pack_test.cpp
  event_loop_test.cpp
  trivial_loop.hpp trivial_loop.cpp
  spy_test.cpp uv_test.cpp
  uv_loop_core_test.cpp
  queue_test.cpp
  )
  
target_link_libraries (loop_test  
  asyncio 
  uv
  pthread
  )

ParseAndAddCatchTests(loop_test)