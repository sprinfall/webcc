#include <filesystem>

#include "gtest/gtest.h"

#include "webcc/logger.h"

int main(int argc, char* argv[]) {
  // Set webcc::LOG_CONSOLE to enable logging.
  WEBCC_LOG_INIT("", 0);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
