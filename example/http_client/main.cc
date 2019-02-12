#include <iostream>

#include "webcc/http_client.h"
#include "webcc/logger.h"

static void PrintError(const webcc::HttpClient& client) {
  std::cout << webcc::DescribeError(client.error());
  if (client.timed_out()) {
    std::cout << " (timed out)";
  }
  std::cout << std::endl;
}

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  auto request = webcc::HttpRequest::Make(webcc::kHttpGet, "/get",
                                          "httpbin.org");

  webcc::HttpClient client;

  if (client.Request(*request)) {
    std::cout << client.response_content() << std::endl;
  } else {
    PrintError(client);
  }

  return 0;
}
