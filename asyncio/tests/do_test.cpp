#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <cstdlib>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {
  // global setup...
  if (const char *env_p = std::getenv("LOG_DEBUG")) {
    spdlog::set_level(spdlog::level::debug);
  }

  int result = Catch::Session().run(argc, argv);

  // global clean-up...

  return (result < 0xff ? result : 0xff);
}

// DO NOT TESTS HERE!