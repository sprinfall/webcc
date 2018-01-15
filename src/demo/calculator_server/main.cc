#include <iostream>
#include "csoap/http_server.h"
#include "calculator_service.h"

static void PrintHelp(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <port>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " 8080" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    PrintHelp(argv[0]);
    return 1;
  }

  const char* host = "0.0.0.0";  // TODO

  try {
    csoap::HttpServer server(host, argv[1]);

    csoap::SoapServicePtr service(new CalculatorService);
    server.RegisterService(service);

    server.Run();

  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
