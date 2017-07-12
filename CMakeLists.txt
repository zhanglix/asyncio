cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project (asyncio)

set(CMAKE_MODULE_PATH 
  ${CMAKE_MODULE_PATH} 
  "${CMAKE_SOURCE_DIR}/cmake.modules/"
  "/usr/local/opt/llvm/lib/cmake/llvm/"
  "/usr/local/opt/llvm/lib/cmake/clang/"
) 

find_package(LLVM REQUIRED CONFIG)
set (CMAKE_C_COMPILER /usr/local/opt/llvm/bin/clang)
set (CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)

# The version number.
set (ASYNCIO_VERSION_MAJOR 0)
set (ASYNCIO_VERSION_MINOR 0)

configure_file (
  "${PROJECT_SOURCE_DIR}/asyncio/config.hpp.in"
  "${PROJECT_BINARY_DIR}/asyncio/config.hpp"
  )
add_compile_options(
  -stdlib=libc++	
  -std=c++14 
  -fsanitize=address 
  -fno-omit-frame-pointer 
  -fcoroutines-ts
  -v)

SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -v -Wl,-rpath,/usr/local/opt/llvm/lib" )

include_directories(
  "${PROJECT_BINARY_DIR}" 
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/contrib/tests"  
  "/usr/local/opt/llvm/include"
)
link_directories(
  "/usr/local/opt/llvm/lib"
)
set (EXTRA_LIBS -lpthread -luv)
enable_testing(true)
include(ParseAndAddCatchTests)  
set(AdditionalCatchParameters ${AdditionalCatchParameters} --rng-seed time)
add_subdirectory(asyncio)