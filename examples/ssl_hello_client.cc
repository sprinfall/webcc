// examples/ssl_hello_client.cc
// See `ssl_hello_server` for the server program.

#include <cassert>
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: ssl_hello_client <cert_file>" << std::endl;
    return 1;
  }

  std::string cert_file = argv[1];

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  // Don't use SslVerify::kHostName as the SSL verification!
  if (!webcc::ClientSession::AddSslContext("default", cert_file,
                                           webcc::SslVerify::kDefault)) {
    std::cerr << "Failed to add the certificate!" << std::endl;
    return 1;
  }

  webcc::ClientSession session;
  session.KeepAlive(false);

  try {
    session.Send(WEBCC_GET("https://localhost:8080/hello")());

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return 1;
  }

  return 0;
}
