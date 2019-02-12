#include <iostream>

#include "boost/asio/io_context.hpp"

#include "webcc/http_async_client.h"
#include "webcc/logger.h"

// TODO: The program blocks during read response.
//   Only HttpBin.org has this issue.

static void Test(boost::asio::io_context& io_context) {
  auto request = webcc::HttpRequest::Make(webcc::kHttpGet, "/get",
                                          "httpbin.org");

  webcc::HttpAsyncClientPtr client{
    new webcc::HttpAsyncClient(io_context)
  };

  client->SetTimeout(3);

  // Response callback.
  auto callback = [](webcc::HttpResponsePtr response, webcc::Error error,
                     bool timed_out) {
    if (error == webcc::kNoError) {
      std::cout << response->content() << std::endl;
    } else {
      std::cout << DescribeError(error);
      if (timed_out) {
        std::cout << " (timed out)";
      }
      std::cout << std::endl;
    }
  };

  client->Request(request, callback);
}

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  boost::asio::io_context io_context;

  //Test(io_context);
  Test(io_context);

  io_context.run();

  return 0;
}
