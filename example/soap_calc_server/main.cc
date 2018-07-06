#include <iostream>

#include "webcc/logger.h"
#include "webcc/soap_server.h"

#include "example/soap_calc_server/calc_service.h"

static void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <port>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " 8080" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));
  std::size_t workers = 2;

  try {
    webcc::SoapServer server(port, workers);
    server.Bind(std::make_shared<CalcService>(), "/calculator");
    server.Run();
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
