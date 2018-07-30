#include <iostream>

#include "boost/asio/io_context.hpp"

#include "webcc/http_async_client.h"
#include "webcc/logger.h"

// In order to test this client, create a file index.html whose content is
// simply "Hello, World!", then start a HTTP server with Python 3:
//     $ python -m http.server
// The default port number should be 8000.

void Test(boost::asio::io_context& io_context) {
  webcc::HttpRequestPtr request(new webcc::HttpRequest());

  request->set_method(webcc::kHttpGet);
  request->set_url("/index.html");
  request->SetHost("localhost", "8000");
  request->UpdateStartLine();

  webcc::HttpAsyncClientPtr client(new webcc::HttpAsyncClient(io_context));

  // Response handler.
  auto handler = [](std::shared_ptr<webcc::HttpResponse> response,
                    webcc::Error error,
                    bool timed_out) {
    if (error == webcc::kNoError) {
      std::cout << response->content() << std::endl;
    } else {
      std::cout << webcc::DescribeError(error);
      if (timed_out) {
        std::cout << " (timed out)";
      }
      std::cout << std::endl;
    }
  };

  client->Request(request, handler);
}

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  boost::asio::io_context io_context;

  Test(io_context);
  Test(io_context);
  Test(io_context);

  io_context.run();

  return 0;
}
