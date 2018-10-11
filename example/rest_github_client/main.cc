#include <iostream>

#include "webcc/rest_ssl_client.h"
#include "webcc/logger.h"

void Test() {
  webcc::RestSslClient client("api.github.com");

  if (client.Get("/events")) {
    std::cout << client.response()->content() << std::endl;
  } else {
    std::cout << webcc::DescribeError(client.error());
    if (client.timed_out()) {
      std::cout << " (timed out)";
    }
    std::cout << std::endl;
  }
}

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  Test();

  return 0;
}
