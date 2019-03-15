#include <functional>
#include <iostream>
#include <string>

#include "webcc/logger.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"
#include "webcc/soap_server.h"
#include "webcc/soap_service.h"

// -----------------------------------------------------------------------------

class CalcService : public webcc::SoapService {
public:
  CalcService() = default;
  ~CalcService() override = default;

  bool Handle(const webcc::SoapRequest& soap_request,
              webcc::SoapResponse* soap_response) final;
};

bool CalcService::Handle(const webcc::SoapRequest& soap_request,
                         webcc::SoapResponse* soap_response) {
  double x = 0.0;
  double y = 0.0;
  try {
    x = std::stod(soap_request.GetParameter("x"));
    y = std::stod(soap_request.GetParameter("y"));
  } catch (const std::exception& e) {
    LOG_ERRO("SoapParameter cast error: %s", e.what());
    return false;
  }

  const std::string& operation = soap_request.operation();

  LOG_INFO("Soap operation '%s': %.2f, %.2f", operation.c_str(), x, y);

  std::function<double(double, double)> calc;

  if (operation == "add") {
    calc = [](double x, double y) { return x + y; };

  } else if (operation == "subtract") {
    calc = [](double x, double y) { return x - y; };

  } else if (operation == "multiply") {
    calc = [](double x, double y) { return x * y; };

  } else if (operation == "divide") {
    calc = [](double x, double y) { return x / y; };

    if (y == 0.0) {
      LOG_ERRO("Cannot divide by 0.");
      return false;
    }
  } else {
    LOG_ERRO("Operation '%s' is not supported.", operation.c_str());
    return false;
  }

  if (!calc) {
    return false;
  }

  double result = calc(x, y);

  soap_response->set_service_ns({
    "cal",
    "http://www.example.com/calculator/"
  });

  soap_response->set_operation(soap_request.operation());
  soap_response->set_simple_result("Result", std::to_string(result), false);

  return true;
}

// -----------------------------------------------------------------------------

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

    // Customize response XML format.
    server.set_format_raw(false);
    server.set_indent_str("  ");

    server.Bind(std::make_shared<CalcService>(), "/calculator");
    server.Run();
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
