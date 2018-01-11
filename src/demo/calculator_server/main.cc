#include <iostream>
#include "csoap/http_server.h"
#include "calculator_service.h"

// TODO: Why need an address for a server?
static void PrintHelp(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <address> <port>" << std::endl;
  std::cout << "  For IPv4, try:" << std::endl;
  std::cout << "    " << argv0 << " 0.0.0.0 80" << std::endl;
  std::cout << "  For IPv6, try:" << std::endl;
  std::cout << "     " << argv0 << " 0::0 80" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    PrintHelp(argv[0]);
    return 1;
  }

  try {
    csoap::HttpServer server(argv[1], argv[2]);

    csoap::SoapServicePtr service(new CalculatorService);
    server.RegisterService(service);

    server.Run();
  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
