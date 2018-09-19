cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project (asyncio)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/cmake.modules/"
)

# find_package(LLVM 5 REQUIRED CONFIG)

# message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
# message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# set (CMAKE_C_COMPILER ${LLVM_TOOLS_BINARY_DIR}/clang)
# set (CMAKE_CXX_COMPILER ${LLVM_TOOLS_BINARY_DIR}/clang++)

# The version number.
set (ASYNCIO_VERSION_MAJOR 0)
set (ASYNCIO_VERSION_MINOR 0)

configure_file (
  "${PROJECT_SOURCE_DIR}/asyncio/config.hpp.in"
  "${PROJECT_BINARY_DIR}/asyncio/config.hpp"
  )

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake.modules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake.modules/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake.modules/cmake_uninstall.cmake")

add_compile_options(
  -fPIC
  -std=c++14
  # -fsanitize=address
  # -fno-omit-frame-pointer
  -fcoroutines-ts
  -v)

#SET(ASYNCIO_LINKER_FLAGS "-z notext -stdlib=libc++ -fsanitize=address -v -Wl -Werror")
SET(ASYNCIO_LINKER_FLAGS "-stdlib=libc++ -lc++abi -v -Wl -Werror")
#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -v -Wl,-rpath,/usr/local/opt/llvm/lib" )
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ASYNCIO_LINKER_FLAGS}" )
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ASYNCIO_LINKER_FLAGS}" )
include_directories(
  "/usr/local/include/c++/v1"
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/contrib/tests"
  ${LLVM_INCLUDE_DIRS}
)


message(STATUS "CMAKE_EXE_LINKER_FLAGS:  ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

link_directories(
${LLVM_LIBRARY_DIRS}
)


set (EXTRA_LIBS -lpthread)
enable_testing(true)
include(ParseAndAddCatchTests)
set(AdditionalCatchParameters ${AdditionalCatchParameters} --rng-seed time)
add_subdirectory(asyncio)