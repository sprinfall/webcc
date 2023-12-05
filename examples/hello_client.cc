// examples/hello_client.cc
// See `hello_server` for the server program.

#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::ClientSession session;

  session.set_read_timeout(2);

  try {
    session.Send(WEBCC_GET("http://localhost:8080/hello")());

    // To test the read timeout.
    session.Send(WEBCC_GET("http://localhost:8080/sleep/3")());

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return 1;
  }

  return 0;
}
