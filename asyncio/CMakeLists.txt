add_subdirectory (coro) 
add_subdirectory(loop)
add_subdirectory(utility)
add_subdirectory(examples)

add_library(asyncio SHARED $<TARGET_OBJECTS:loop> $<TARGET_OBJECTS:utility>)
target_link_libraries (asyncio uv)
add_library(asyncio_static STATIC $<TARGET_OBJECTS:loop> $<TARGET_OBJECTS:utility>)

install(TARGETS asyncio LIBRARY DESTINATION lib )
install(TARGETS asyncio_static ARCHIVE DESTINATION lib )

install(
  DIRECTORY ./
  DESTINATION include/asyncio
  FILES_MATCHING PATTERN "*.hpp"
  REGEX "tests" EXCLUDE
  REGEX "examples" EXCLUDE  
  )


install(
  DIRECTORY examples
  DESTINATION share/asyncio
  FILES_MATCHING PATTERN "*.cpp"
  )