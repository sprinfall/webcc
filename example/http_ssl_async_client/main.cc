#include <iostream>

#include "boost/asio/io_context.hpp"

#include "webcc/http_ssl_async_client.h"
#include "webcc/logger.h"

int main(int argc, char* argv[]) {
  std::string host;
  std::string url;

  if (argc != 3) {
    host = "www.boost.org";
    url = "/LICENSE_1_0.txt";
  } else {
    host = argv[1];
    url = argv[2];
  }

  std::cout << "Host: " << host << std::endl;
  std::cout << "URL:  " << url << std::endl;
  std::cout << std::endl;

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  boost::asio::io_context io_context;

  // Leave port to default value.
  auto request = webcc::HttpRequest::Make(webcc::kHttpGet, url, host);

  // Verify the certificate of the peer or not.
  // See HttpSslClient::Request() for more details.
  bool ssl_verify = false;

  webcc::HttpSslAsyncClientPtr client{
    new webcc::HttpSslAsyncClient{ io_context, 2000, ssl_verify }
  };

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

  io_context.run();

  return 0;
}
