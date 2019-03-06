#include <iostream>

#include "webcc/http_ssl_client.h"
#include "webcc/logger.h"

int main(int argc, char* argv[]) {
  std::string url;

  if (argc != 3) {
    url = "www.boost.org/LICENSE_1_0.txt";
  } else {
    url = argv[1];
  }

  std::cout << "URL:  " << url << std::endl;
  std::cout << std::endl;

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  // Leave port to default value.
  webcc::HttpRequest request(webcc::http::kGet, url);

  request.Prepare();

  // Verify the certificate of the peer or not.
  // See HttpSslClient::Request() for more details.
  bool ssl_verify = false;

  webcc::HttpSslClient client(ssl_verify, 2000);

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
