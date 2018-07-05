#include <iostream>

#include "webcc/logger.h"
#include "webcc/http_client.h"

// In order to test this client, create a file index.html whose content is
// simply "Hello, World!", then start a HTTP server with Python 3:
//     $ python -m http.server
// The default port number should be 8000.

void Test() {
  webcc::HttpRequest request;

  request.set_method(webcc::kHttpGet);
  request.set_url("/index.html");
  request.SetHost("localhost", "8000");

  request.Build();

  webcc::HttpResponse response;

  webcc::HttpClient client;
  if (!client.Request(request)) {
    return;
  }

  std::cout << response.content() << std::endl;
}

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  Test();
  Test();
  Test();

  return 0;
}
