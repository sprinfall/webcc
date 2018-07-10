#include <iostream>

#include "webcc/http_client.h"
#include "webcc/logger.h"

// In order to test this client, create a file index.html whose content is
// simply "Hello, World!", then start a HTTP server with Python 3:
//     $ python -m http.server
// The default port number should be 8000.

void Test() {
  webcc::HttpRequest request;
  request.set_method(webcc::kHttpGet);
  request.set_url("/index.html");
  request.SetHost("localhost", "8000");
  request.UpdateStartLine();

  webcc::HttpClient client;
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
  Test();
  Test();

  return 0;
}
