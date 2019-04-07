#include "webcc/logger.h"

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  LOG_INFO("info");
  LOG_WARN("warn");
  LOG_ERRO("erro");
  LOG_VERB("verb");
  LOG_FATA("fata");
  LOG_INFO("info");

  return 0;
}

