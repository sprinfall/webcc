#include <iostream>

#include "boost/asio/io_context.hpp"

#include "webcc/logger.h"
#include "webcc/async_http_client.h"

// In order to test this client, create a file index.html whose content is
// simply "Hello, World!", then start a HTTP server with Python 3:
//     $ python -m http.server
// The default port number should be 8000.

void Test(boost::asio::io_context& ioc) {
  std::shared_ptr<webcc::HttpRequest> request(new webcc::HttpRequest());

  request->set_method(webcc::kHttpGet);
  request->set_url("/index.html");
  request->SetHost("localhost", "8000");

  request->Build();

  webcc::HttpAsyncClientPtr client(new webcc::AsyncHttpClient(ioc));

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

  boost::asio::io_context ioc;

  Test(ioc);
  Test(ioc);
  Test(ioc);

  ioc.run();

  return 0;
}
