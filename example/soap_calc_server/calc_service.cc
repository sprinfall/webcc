#include "example/soap_calc_server/calc_service.h"

#include <functional>
#include <string>

#include "webcc/logger.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

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

  soap_response->set_soapenv_ns(webcc::kSoapEnvNamespace);
  soap_response->set_service_ns({
    "cal",
    "http://www.example.com/calculator/"
  });

  soap_response->set_operation(soap_request.operation());

  soap_response->set_result_name("Result");
  soap_response->set_result_moved(std::to_string(result), false);

  return true;
}
