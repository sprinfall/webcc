#include "calc_service.h"

#include <functional>
#include <string>

// Sleep several seconds for the client to test timeout control.
#define SLEEP_FOR_TIMEOUT_TEST 0

#if SLEEP_FOR_TIMEOUT_TEST
#include "boost/thread/thread.hpp"
#endif

#include "webcc/logger.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

#if SLEEP_FOR_TIMEOUT_TEST
static const int kSleepSeconds = 3;
#endif

bool CalcService::Handle(const webcc::SoapRequest& soap_request,
                         webcc::SoapResponse* soap_response) {
  double x = 0.0;
  double y = 0.0;
  if (!GetParameters(soap_request, &x, &y)) {
    return false;
  }

  const std::string& op = soap_request.operation();

  LOG_INFO("Soap operation '%s': %.2f, %.2f", op.c_str(), x, y);

  std::function<double(double, double)> calc;

  if (op == "add") {
    calc = [](double x, double y) { return x + y; };

  } else if (op == "subtract") {
    calc = [](double x, double y) { return x - y; };

  } else if (op == "multiply") {
    calc = [](double x, double y) { return x * y; };

  } else if (op == "divide") {
    calc = [](double x, double y) { return x / y; };

    if (y == 0.0) {
      LOG_ERRO("Cannot divide by 0.");
      return false;
    }
  } else {
    LOG_ERRO("Operation '%s' is not supported.", op.c_str());
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
  soap_response->set_result(std::to_string(result));

#if SLEEP_FOR_TIMEOUT_TEST
  LOG_INFO("Sleep %d seconds for the client to test timeout control.",
           kSleepSeconds);
  boost::this_thread::sleep_for(boost::chrono::seconds(kSleepSeconds));
#endif

  return true;
}

bool CalcService::GetParameters(const webcc::SoapRequest& soap_request,
                                double* x,
                                double* y) {
  try {
    *x = std::stod(soap_request.GetParameter("x"));
    *y = std::stod(soap_request.GetParameter("y"));

  } catch (const std::exception& e) {
    LOG_ERRO("Parameter cast error: %s", e.what());
    return false;
  }

  return true;
}
