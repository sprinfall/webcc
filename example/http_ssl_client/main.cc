#include <iostream>

#include "webcc/http_ssl_client.h"
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

  // Leave port to default value.
  webcc::HttpRequest request(webcc::kHttpGet, url, host);

  request.Make();

  // Verify the certificate of the peer or not.
  // See HttpSslClient::Request() for more details.
  bool ssl_verify = false;

  webcc::HttpSslClient client(2000, ssl_verify);

  if (client.Request(request)) {
    //std::cout << client.response()->content() << std::endl;
  } else {
    std::cout << webcc::DescribeError(client.error());
    if (client.timed_out()) {
      std::cout << " (timed out)";
    }
    std::cout << std::endl;
  }

  return 0;
}
