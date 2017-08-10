#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <cstdlib>
#include <spdlog/spdlog.h>

#define ENABLE_ASYNCIO_LOG
#include "log.hpp"
using namespace Catch;
struct TestEventLogger : TestEventListenerBase {

  using TestEventListenerBase::TestEventListenerBase; // inherit constructor
  // The whole test run, starting and ending
  // void testRunStarting(TestRunInfo const &testRunInfo) {}
  // void testRunEnded(TestRunStats const &testRunStats) {}

  // Test cases starting and ending
  void testCaseStarting(TestCaseInfo const &testInfo) {
    LOG_DEBUG("Case start: {}", testInfo.name);
  }
  void testCaseEnded(TestCaseStats const &testCaseStats) {
    LOG_DEBUG("Case ended: {}", testCaseStats.testInfo.name);
  }

  // Sections starting and ending
  void sectionStarting(SectionInfo const &sectionInfo) {
    LOG_DEBUG("Section start: {}", sectionInfo.name);
  }
  void sectionEnded(SectionStats const &sectionStats) {
    LOG_DEBUG("Section ended: {}", sectionStats.sectionInfo.name);
  }

  // // Assertions before/ after
  // void assertionStarting(AssertionInfo const &assertionInfo) {}
  // bool assertionEnded(AssertionStats const &assertionStats) {}

  // A test is being skipped (because it is "hidden")
  // void skipTest(TestCaseInfo const &testInfo) {}
};

CATCH_REGISTER_LISTENER(TestEventLogger)

int main(int argc, char *argv[]) {
  // global setup...
  if (const char *env_p = std::getenv("LOG_DEBUG")) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  int result = Session().run(argc, argv);

  // global clean-up...

  return (result < 0xff ? result : 0xff);
}

// DO NOT TESTS HERE!