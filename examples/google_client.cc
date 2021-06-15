// google_client.cc
// This example sends a GET request to https://www.google.com/.

// NOTE:
// - The response body is chunked
// - The Google server requires the client to call `SSL_set_tlsext_host_name()`
//   or the SSL handshake would fail. This is different from other HTTPS servers
//   like api.github.com or httpbin.org.

#include <cassert>
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::ClientSession session;

  try {
    auto r = session.Send(WEBCC_GET("https://www.google.com/")());

    std::cout << std::endl;
    std::cout << r->status() << std::endl;
    std::cout << std::endl;
    std::cout << r->data() << std::endl;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return 1;
  }

  return 0;
}
