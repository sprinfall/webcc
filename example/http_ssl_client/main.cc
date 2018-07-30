#include <iostream>

#include "webcc/http_ssl_client.h"
#include "webcc/logger.h"

void Test() {
  webcc::HttpRequest request;
  request.set_method(webcc::kHttpGet);
  request.set_url("/LICENSE_1_0.txt");
  request.SetHost("www.boost.org", "443");
  request.UpdateStartLine();

  webcc::HttpSslClient client;
  if (client.Request(request)) {
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
